#include "undo_journal.h"
#include "block_write_plan.h"
#include "crc32.h"
#include "file_support.h"
#include "operation_checkpoint.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <limits>
#include <portable_file_io.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace snfiletools::internal {
namespace {
namespace fs = std::filesystem;
using File = std::unique_ptr<FILE, decltype(&file_close)>;
constexpr std::uint64_t max_document = std::numeric_limits<std::uint32_t>::max();
struct Range {
    std::uint64_t offset, size, journal_offset = 0;
};
struct Lock {
    FILE *file;
    explicit Lock(FILE *value) : file(value) {
        if (file_lock_exclusive(file) != 0)
            throw io_error("Document is busy or cannot be locked");
    }
    ~Lock() { file_unlock(file); }
};
std::string number(std::uint64_t value, unsigned bytes = 8) {
    std::string result(bytes, '\0');
    for (unsigned i = 0; i < bytes; ++i)
        result[i] = static_cast<char>((value >> (8 * i)) & 255);
    return result;
}
std::uint64_t number(const char *value, unsigned bytes = 8) {
    std::uint64_t result = 0;
    for (unsigned i = 0; i < bytes; ++i)
        result |= std::uint64_t(static_cast<unsigned char>(value[i])) << (8 * i);
    return result;
}
fs::path journal_directory;
struct Crc {
    std::uint32_t value = 0xffffffffu;
    void add(const char *data, std::size_t size) { value = crc32_update(value, data, size); }
    std::uint32_t sum() const { return ~value; }
};
void write(FILE *file, const char *data, std::size_t size) {
    if (std::fwrite(data, 1, size, file) != size)
        throw io_error("Failed to write recovery data");
}
void write(FILE *file, const std::string &value) {
    write(file, value.data(), value.size());
}
void read(FILE *file, char *data, std::size_t size) {
    if (std::fread(data, 1, size, file) != size)
        throw io_error("Read recovery journal", std::ferror(file) ? errno : EIO);
}
void flush_stream(FILE *file) {
    if (std::fflush(file) != 0)
        throw io_error("Failed to flush recovery transaction");
}
bool sidecar_exists(const fs::path &path) {
    const auto status = fs::symlink_status(path);
    if (status.type() == fs::file_type::not_found)
        return false;
    if (status.type() != fs::file_type::regular)
        throw std::runtime_error("Recovery sidecar is not a regular file");
    return true;
}
std::vector<Range> overwritten(const BlockWritePlan &plan, std::uint64_t size) {
    std::vector<Range> ranges;
    for (const auto &span : plan.spans) {
        if ((span.copy && span.source == span.destination) || span.destination >= size)
            continue;
        const auto count = std::min(span.size, size - span.destination);
        if (count != 0)
            ranges.push_back({span.destination, count});
    }
    if (plan.size < size)
        ranges.push_back({plan.size, size - plan.size});
    std::sort(ranges.begin(), ranges.end(), [](const auto &a, const auto &b) { return a.offset < b.offset; });
    std::vector<Range> merged;
    for (const auto &range : ranges) {
        if (!merged.empty() && range.offset <= merged.back().offset + merged.back().size)
            merged.back().size =
                std::max(merged.back().offset + merged.back().size, range.offset + range.size) - merged.back().offset;
        else
            merged.push_back(range);
    }
    return merged;
}
struct Prepared {
    std::uint64_t device, identity, original_size, final_size;
    std::vector<Range> ranges;
    bool committed = false;
};
Prepared inspect(FILE *file) {
    file_offset_t bytes = 0;
    if (file_size(file, &bytes) != 0)
        throw io_error("Inspect recovery journal size");
    if (bytes < 60)
        throw std::runtime_error("Incomplete recovery journal");
    std::array<char, 48> header{};
    read(file, header.data(), header.size());
    if (std::string_view(header.data(), 8) != "SNUNDO01")
        throw std::runtime_error("Unknown recovery journal version");
    Crc crc;
    crc.add(header.data(), header.size());
    Prepared result{number(header.data() + 8),
                    number(header.data() + 16),
                    number(header.data() + 24),
                    number(header.data() + 32),
                    {}};
    const auto count = number(header.data() + 40);
    if (result.original_size > max_document || result.final_size > max_document ||
        count > std::uint64_t(bytes - 60) / 17)
        throw std::runtime_error("Invalid recovery journal bounds");
    std::vector<char> buffer(io_workspace_bytes);
    std::uint64_t position = 48, end = 0;
    for (std::uint64_t i = 0; i < count; ++i) {
        std::array<char, 16> entry{};
        read(file, entry.data(), entry.size());
        crc.add(entry.data(), entry.size());
        position += entry.size();
        const Range range{number(entry.data()), number(entry.data() + 8), position};
        if (range.size == 0 || range.offset < end || range.offset > result.original_size ||
            range.size > result.original_size - range.offset || position > std::uint64_t(bytes) - 12 ||
            range.size > std::uint64_t(bytes) - 12 - position)
            throw std::runtime_error("Invalid recovery range");
        for (std::uint64_t done = 0; done < range.size;) {
            const auto n = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), range.size - done));
            read(file, buffer.data(), n);
            crc.add(buffer.data(), n);
            done += n;
        }
        position += range.size;
        end = range.offset + range.size;
        result.ranges.push_back(range);
    }
    std::array<char, 12> ready{};
    read(file, ready.data(), ready.size());
    position += ready.size();
    if (std::string_view(ready.data(), 8) != "PREPARED" || number(ready.data() + 8, 4) != crc.sum())
        throw std::runtime_error("Recovery journal checksum mismatch; original bytes have not been restored");
    const auto extra = std::uint64_t(bytes) - position;
    if (extra > 16)
        throw std::runtime_error("Unexpected recovery journal data");
    if (extra == 16) {
        std::array<char, 16> commit{};
        read(file, commit.data(), commit.size());
        result.committed = std::string_view(commit.data(), 8) == "COMMIT01" &&
                           number(commit.data() + 8, 4) == crc.sum() &&
                           number(commit.data() + 12, 4) == std::uint32_t(~crc.sum());
    }
    // An absent, partial, or torn commit marker always rolls back. No successful
    // write is reported until the complete marker itself has been flushed.
    return result;
}
} // namespace

struct UndoJournal::State {
    Lock lock;
    fs::path active, pending;
    File journal{nullptr, &file_close};
    std::uint32_t checksum = 0;
    bool committed = false;
    State(FILE *file, const fs::path &active_path)
        : lock(file), active(active_path), pending(fs::path(active).concat(".pending")) {}
};
bool UndoJournal::configure(const fs::path &directory) {
    if (directory.empty())
        throw std::invalid_argument("Journal directory is empty");
    fs::create_directories(directory);
    auto resolved = fs::canonical(directory);
    if (!fs::is_directory(resolved))
        throw std::invalid_argument("Journal location is not a directory");
    if (!journal_directory.empty() && journal_directory != resolved && !fs::is_empty(journal_directory))
        throw std::logic_error("Cannot change journal directory while recovery files remain");
    if (journal_directory == resolved)
        return false;
    journal_directory = std::move(resolved);
    return true;
}
bool UndoJournal::configured() {
    return !journal_directory.empty();
}
static fs::path journal_for_canonical(const fs::path &canonical) {
    if (journal_directory.empty())
        return fs::path(canonical).concat(".snundo"); // Legacy reads/recovery only.
    std::uint64_t hash = 14695981039346656037ull;
    for (char8_t c : canonical.generic_u8string()) {
        hash ^= c;
        hash *= 1099511628211ull;
    }
    std::ostringstream name;
    name << std::hex << std::setw(16) << std::setfill('0') << hash << ".snundo";
    return journal_directory / name.str();
}
fs::path UndoJournal::journal_path(const fs::path &path) {
    return journal_for_canonical(fs::canonical(path));
}
UndoJournal::UndoJournal(FILE *document, const fs::path &path, const BlockWritePlan &plan, const file_state_t &expected,
                         std::span<char> workspace, const fs::path &cached_journal)
    : state_(std::make_unique<State>(document, cached_journal.empty() ? journal_path(path) : cached_journal)) {
    if (!configured())
        throw std::logic_error("Configure a persistent private journal directory before writing");
    auto &s = *state_;
    if (sidecar_exists(s.active) || sidecar_exists(s.pending))
        throw std::runtime_error("A previous write requires recovery");

    file_state_t original{};
    if (file_state(document, &original) != 0)
        throw io_error("Cannot inspect document for recovery");
    if (!same_state(original, expected))
        throw std::runtime_error("Document changed before the write lock was acquired");
    const auto ranges = overwritten(plan, original.size);
    s.journal.reset(file_open(s.pending, "w+bx"));
    if (!s.journal)
        throw io_error("Cannot exclusively create recovery journal");
    // Keep small range descriptors together; this is only a buffering hint.
    // The same preparation/commit flush boundaries still govern recovery.
    (void)std::setvbuf(s.journal.get(), nullptr, _IOFBF, stream_buffer_bytes);
    operation_checkpoint("journal.pending");
    Crc crc;
    auto record = [&](const char *data, std::size_t count) {
        write(s.journal.get(), data, count);
        crc.add(data, count);
    };
    const auto header = std::string("SNUNDO01") + number(original.device) + number(original.identity) +
                        number(original.size) + number(plan.size) + number(ranges.size());
    record(header.data(), header.size());
    std::vector<char> owned;
    if (workspace.empty() && !ranges.empty()) {
        owned.resize(plan.workspace_size(original.size));
        workspace = owned;
    }
    auto buffer = workspace;
    for (const auto &range : ranges) {
        const auto entry = number(range.offset) + number(range.size);
        record(entry.data(), entry.size());
        for (std::uint64_t done = 0; done < range.size;) {
            const auto n = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), range.size - done));
            // Preparation precedes all mutation, and earlier transactions flush
            // their output. Positioned reads need no per-range stream seek.
            if (file_read_exact_at(document, static_cast<file_offset_t>(range.offset + done), buffer.data(), n))
                throw io_error("Cannot read original bytes for recovery");
            record(buffer.data(), n);
            done += n;
            operation_checkpoint("journal.chunk");
        }
    }
    s.checksum = crc.sum();
    write(s.journal.get(), "PREPARED" + number(s.checksum, 4));
    flush_stream(s.journal.get());
    operation_checkpoint("journal.synced");
    file_state_t current{}, at_path{};
    if (file_state(document, &current) != 0)
        throw io_error("Inspect document after journal preparation");
    if (file_path_state(path, &at_path) != 0)
        throw io_error("Inspect document path after journal preparation");
    if (!same_state(original, current) || !same_state(original, at_path))
        throw std::runtime_error("Document changed while preparing recovery");
#ifdef _WIN32
    // CRT journal streams do not share delete access. Keep the document lock
    // throughout close/rename/reopen; no document mutation has begun yet.
    if (file_close(s.journal.release()) != 0)
        throw io_error("Close prepared recovery journal");
    operation_checkpoint("journal.closed");
#endif
    fs::rename(s.pending, s.active);
    operation_checkpoint("journal.published");
#ifdef _WIN32
    s.journal.reset(file_open(s.active, "r+b"));
    if (!s.journal || file_seek(s.journal.get(), 0, SEEK_END))
        throw io_error("Reopen prepared recovery journal");
    operation_checkpoint("journal.reopened");
#endif
}
UndoJournal::~UndoJournal() = default; // Leave failed transactions available for recovery.
void UndoJournal::commit() {
    auto &s = *state_;
    // execute() has already flushed the document. A partial commit marker
    // is safe: recovery will restore the saved original ranges.
    write(s.journal.get(), "COMMIT01");
    operation_checkpoint("journal.commit_partial");
    write(s.journal.get(), number(s.checksum, 4) + number(std::uint32_t(~s.checksum), 4));
    flush_stream(s.journal.get());
    s.committed = true;
    operation_checkpoint("journal.committed");
    s.journal.reset();
    fs::remove(s.active);

    operation_checkpoint("journal.removed");
}

bool UndoJournal::committed_on_disk(const fs::path &active) {
    if (!sidecar_exists(active))
        return false;
    File journal(file_open(active, "rb"), &file_close);
    if (!journal)
        throw io_error("Cannot inspect failed transaction");
    return inspect(journal.get()).committed;
}
bool UndoJournal::committed() const noexcept {
    return state_->committed;
}
bool UndoJournal::recover(const fs::path &path, fs::path *resolved_journal) {
    const auto canonical = fs::canonical(path);
    const auto legacy = fs::path(canonical).concat(".snundo");
    const auto active = journal_for_canonical(canonical);
    if (resolved_journal)
        *resolved_journal = active;
    bool done = recover_one(path, legacy);
    if (active != legacy)
        done = recover_one(path, active) || done;
    // A hard-link alias has a different canonical name but the same identity.
    // This scan is only on opening/recovery, never a healthy cached read.
    if (!journal_directory.empty()) {
        file_state_t target{};
        if (file_path_state(path, &target))
            throw io_error("Cannot inspect recovery target");
        for (const auto &entry : fs::directory_iterator(journal_directory)) {
            if (entry.path() == active || entry.path().extension() != ".snundo")
                continue;
            File f(file_open(entry.path(), "rb"), &file_close);
            std::array<char, 24> header{};
            if (!f || std::fread(header.data(), 1, header.size(), f.get()) != header.size())
                continue;
            if (std::string_view(header.data(), 8) == "SNUNDO01" && number(header.data() + 8) == target.device &&
                number(header.data() + 16) == target.identity) {
                f.reset();
                done = recover_one(path, entry.path()) || done;
            }
        }
    }
    return done;
}
bool UndoJournal::recover_one(const fs::path &path, const fs::path &active) {
    const fs::path pending = fs::path(active).concat(".pending");
    if (!sidecar_exists(active) && !sidecar_exists(pending))
        return false;
    File document(file_open_shared(path, "r+b"), &file_close);
    if (!document)
        throw io_error("Cannot open document for recovery");
    Lock lock(document.get());
    // Recheck under the document lock: another writer may have finished meanwhile.
    if (sidecar_exists(active)) {
        File journal(file_open(active, "rb"), &file_close);
        if (!journal)
            throw io_error("Cannot read recovery journal");
        const auto saved = inspect(journal.get()); // Full checksum validation BEFORE any mutation.
        file_state_t current{};
        if (file_state(document.get(), &current) != 0)
            throw io_error("Inspect document during recovery");
        if (current.device != saved.device || current.identity != saved.identity)
            throw std::runtime_error("Recovery journal belongs to a different document; refusing to restore");
        if (saved.committed && current.size != saved.final_size)
            throw std::runtime_error("Committed document length has changed; retaining recovery journal");
        if (!saved.committed) {
            std::vector<char> buffer(io_workspace_bytes);
            for (const auto &range : saved.ranges) {
                for (std::uint64_t done = 0; done < range.size;) {
                    const auto n = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), range.size - done));
                    block_read_exact(journal.get(), range.journal_offset + done, buffer.data(), n);
                    if (file_seek(document.get(), static_cast<file_offset_t>(range.offset + done), SEEK_SET) != 0)
                        throw io_error("Cannot seek during recovery");
                    write(document.get(), buffer.data(), n);
                    done += n;
                    operation_checkpoint("recovery.chunk");
                }
            }
            if (file_truncate(document.get(), static_cast<file_offset_t>(saved.original_size)) != 0)
                throw io_error("Cannot restore original document length");
            operation_checkpoint("recovery.truncated");
            flush_stream(document.get());
            operation_checkpoint("recovery.synced");
        }
        journal.reset();
        fs::remove(active);

        operation_checkpoint("recovery.removed");
    }
    // Pending journals were never published: the document could not yet have
    // been modified by that transaction, even if the pending file is incomplete.
    if (sidecar_exists(pending)) {
        fs::remove(pending);
    }
    return true;
}
} // namespace snfiletools::internal
