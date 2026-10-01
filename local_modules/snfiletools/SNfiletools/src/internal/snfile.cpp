#include "snfile.h"
#include "block_index.h"
#include "block_write_plan.h"
#include "file_support.h"
#include "operation_checkpoint.h"
#include "parallel_read_hooks.h"
#include "read_state_pool.h"
#include "undo_journal.h"
#include <algorithm>
#include <array>
#include <filesystem>
#include <limits>
#include <stdexcept>
#if (defined(__ANDROID__) || defined(SNFILETOOLS_TEST_PARALLEL_READS)) && !defined(_WIN32)
#include <cerrno>
#include <exception>
#include <thread>
#include <unistd.h>
#endif

namespace {
template <class String> void ascii_to_lower(String &str) noexcept {
    for (auto &character : str) {
        if (character >= 'A' && character <= 'Z') {
            character -= 'A';
            character += 'a';
        }
    }
}

std::filesystem::path normalize_document_path(const std::filesystem::path &path) {
    if (path.empty())
        throw std::runtime_error("Path is empty");
    // Most calls reuse an already-normal absolute path. Scan its native text
    // without allocating path components before taking the normalization path.
    const auto &native = path.native();
    const auto separator = [](auto c) { return c == '/' || c == std::filesystem::path::preferred_separator; };
    bool needs_cleanup = !path.is_absolute();
    for (std::size_t i = 0; !needs_cleanup && i < native.size(); ++i) {
        if (!separator(native[i]))
            continue;
        auto next = i + 1;
        if (next < native.size() && separator(native[next])) {
            needs_cleanup = true;
        } else if (next < native.size() && native[next] == '.') {
            ++next;
            if (next < native.size() && native[next] == '.')
                ++next;
            needs_cleanup = next == native.size() || separator(native[next]);
        }
    }
    if (!needs_cleanup)
        return path;
    auto absolute = path.is_absolute() ? path : std::filesystem::absolute(path);
    // '..' across a symlink must retain filesystem semantics. The common path
    // (including '.') needs no filesystem queries for normalization.
    const auto lexical = absolute.lexically_normal();
    for (const auto &part : absolute) {
        if (part == "..") {
            const auto resolved = std::filesystem::canonical(absolute);
            std::error_code error;
            // Keep unrelated symlink spellings intact when lexical parent
            // removal reaches the same file (e.g. macOS /var versus /private/var).
            if (std::filesystem::equivalent(lexical, resolved, error) && !error)
                return lexical;
            return resolved;
        }
    }
    return lexical;
}
} // namespace

namespace snfiletools::internal {
BlockHandle CreatedBlock::handle() const {
    if (!state_ || state_->status != CreatedState::ready)
        throw std::logic_error("Created block handle is not ready (pending, failed, or cancelled)");
    return state_->handle;
}
SNfile::SNfile(std::filesystem::path path, std::filesystem::path data_path)
    : file_type_(path == data_path ? FileType::note : FileType::mark), file_path_(std::move(path)),
      data_path_(std::move(data_path)) {
    if (file_type_ == FileType::mark &&
        (!std::filesystem::is_regular_file(file_path_) || !std::filesystem::is_regular_file(data_path_)))
        throw std::runtime_error("Both PDF and mark files must exist");
    UndoJournal::recover(data_path_, &journal_path_);
    file_.reset(file_open_shared(data_path_, "rb"));
    if (!file_)
        throw io_error("Failed to open file");
    if (file_state(file_.get(), &state_) != 0)
        throw io_error("Failed to inspect file state");
}

void SNfile::touch() const noexcept {
    const auto position =
        std::find_if(open_snfiles.begin(), open_snfiles.end(), [this](const auto &file) { return file.get() == this; });
    if (position != open_snfiles.end()) {
        std::rotate(open_snfiles.begin(), position, position + 1);
    }
}

std::int64_t SNfile::get_size() const {
    touch();
    // Only a previously mutable writer can enter this recovery state.
    if (write_failed_)
        const_cast<SNfile *>(this)->refresh_if_changed();
    return state_.size;
}

void SNfile::refresh_if_changed() {
    // Close a failed update stream BEFORE recovery, so buffered writes cannot
    // later flush over restored bytes. A failed recovery can be retried.
    if (write_failed_ && file_ != nullptr) {
        file_.reset();
    }
    // The companion is an existence constraint, never a PDF stream or cache.
    if (file_type_ == FileType::mark && !std::filesystem::is_regular_file(file_path_))
        throw std::runtime_error("PDF companion is missing or not a regular file");
    file_state_t current{};
    if (file_path_state(data_path_, &current) != 0)
        throw io_error("Inspect document path during refresh");
    if (file_type_ == FileType::mark && !same_state(current, state_) && !std::filesystem::is_regular_file(data_path_))
        throw std::runtime_error("Mark companion is not a regular file");
    const bool recovered =
        (write_failed_ || !file_ || !same_state(current, state_)) && UndoJournal::recover(data_path_, &journal_path_);
    if (recovered || file_ == nullptr || !same_state(current, state_)) {
        File replacement(file_open_shared(data_path_, "rb"), &file_close);
        if (!replacement)
            throw io_error("Reopen changed document");
        if (file_state(replacement.get(), &current) != 0)
            throw io_error("Inspect reopened document");
        block_index_.reset();
        file_ = std::move(replacement);
        writable_ = false;
        write_failed_ = false;
        state_ = current;
    }
}

void SNfile::ensure_block_index() {
    if (write_failed_ || !file_)
        refresh_if_changed();
    if (!block_index_)
        block_index_ = std::make_unique<BlockIndex>(file_.get());
}

std::vector<BlockInfo> SNfile::list_blocks() {
    touch();
    ensure_block_index();
    return block_index_->describe();
}

BlockInfo SNfile::get_block_info(BlockHandle handle) {
    touch();
    ensure_block_index();
    return block_index_->describe(handle);
}
std::vector<BlockInfo> SNfile::get_children(BlockHandle handle) {
    touch();
    ensure_block_index();
    return block_index_->children(handle);
}
void SNfile::cancel_pending() noexcept {
    for (auto &request : reads_)
        request.result->status = ReadState::cancelled;
    reads_.clear();
    writes_.clear();
    for (auto &request : creations_)
        request.result->status = CreatedState::cancelled;
    creations_.clear();
    attachments_.clear();
}

ReadResult SNfile::queue_read(BlockHandle handle, std::optional<ReadRange> range) {
    try {
        auto result = ReadStatePool::acquire();
        reads_.push_back({handle, range, result});
        return ReadResult(std::move(result));
    } catch (...) {
        cancel_pending();
        throw;
    }
}
ReadResult SNfile::read(BlockHandle handle) {
    touch();
    return queue_read(handle, {});
}
ReadResult SNfile::read(BlockHandle handle, ReadRange range) {
    touch();
    return queue_read(handle, range);
}
void SNfile::queue_write(BlockHandle handle, std::optional<WriteRange> range, ByteBuffer bytes) {
    try {
        writes_.push_back({handle, range, std::move(bytes)});
    } catch (...) {
        cancel_pending();
        throw;
    }
}
void SNfile::write(BlockHandle handle, std::span<const char> bytes) {
    touch();
    try {
        queue_write(handle, {}, ByteBuffer(bytes.begin(), bytes.end()));
    } catch (...) {
        cancel_pending();
        throw;
    }
}
void SNfile::write(BlockHandle handle, WriteRange range, std::span<const char> bytes) {
    touch();
    try {
        queue_write(handle, range, ByteBuffer(bytes.begin(), bytes.end()));
    } catch (...) {
        cancel_pending();
        throw;
    }
}
void SNfile::write(BlockHandle handle, ByteBuffer &&bytes) {
    touch();
    queue_write(handle, {}, std::move(bytes));
}
void SNfile::write(BlockHandle handle, WriteRange range, ByteBuffer &&bytes) {
    touch();
    queue_write(handle, range, std::move(bytes));
}
CreatedBlock SNfile::create(BlockKind kind, std::span<const char> bytes) {
    touch();
    try {
        return create(kind, ByteBuffer(bytes.begin(), bytes.end()));
    } catch (...) {
        cancel_pending();
        throw;
    }
}
CreatedBlock SNfile::create(BlockKind kind, ByteBuffer &&bytes) {
    touch();
    try {
        auto state = std::make_shared<CreatedState>();
        creations_.push_back({kind, std::move(bytes), state});
        return CreatedBlock(std::move(state));
    } catch (...) {
        cancel_pending();
        throw;
    }
}
void SNfile::attach(BlockTarget parent, std::string key, BlockTarget target) {
    touch();
    try {
        attachments_.push_back({std::move(parent), std::move(key), std::move(target)});
    } catch (...) {
        cancel_pending();
        throw;
    }
}
void SNfile::run() {
    touch();
    auto reads = std::move(reads_);
    auto writes = std::move(writes_);
    auto creations = std::move(creations_);
    auto attachments = std::move(attachments_);
    reads_.clear();
    writes_.clear();
    creations_.clear();
    attachments_.clear();
    // Consume requests on every exit, retaining the allocation for the next
    // batch. Do not replace any new queue created by an observation hook.
    struct RecycleReadQueue {
        std::vector<ReadOperation> &queue, &completed;
        ~RecycleReadQueue() {
            completed.clear();
            if (queue.empty() && queue.capacity() < completed.capacity())
                queue.swap(completed);
        }
    } recycle{reads_, reads};
    try {
        if (!reads.empty() && (!writes.empty() || !creations.empty() || !attachments.empty()))
            throw std::invalid_argument("A run must contain only reads or only writes");
        if (reads.empty() && writes.empty() && creations.empty() && attachments.empty())
            return;
        // Requests can be queued after arbitrary caller work. Check once here,
        // invalidating old handles if the document changed since open/last run.
        refresh_if_changed();
        if (!reads.empty())
            execute_reads(reads);
        else
            execute_writes(writes, creations, attachments);
    } catch (...) {
        for (auto &request : reads) {
            request.result->bytes.clear();
            request.result->status = ReadState::failed;
        }
        for (auto &request : creations)
            request.result->status = CreatedState::failed;
        throw;
    }
}

void SNfile::execute_reads(const std::vector<ReadOperation> &requests) {
    ensure_block_index();
    struct Read {
        std::uint64_t begin, end;
        std::size_t index;
    };
    std::vector<Read> reads;
    reads.reserve(requests.size());
    std::vector<ByteBuffer> results(requests.size());
    for (std::size_t i = 0; i < requests.size(); ++i) {
        const auto &r = requests[i];
        const auto &b = block_index_->get(r.handle);
        const auto pos = r.range ? BlockIndex::position(b, r.range->origin, r.range->offset, true) : 0;
        const auto n = pos >= b.size ? 0 : std::min<std::size_t>(b.size - pos, r.range ? r.range->bytes : b.size);
        // Keep the initialized size for singleton reads so resize need not
        // zero bytes that positioned I/O will overwrite. Pending/failed states
        // never expose this recycled storage to callers.
        results[i] = std::move(r.result->bytes);
        results[i].reserve(n);
        if (!n)
            results[i].clear();
        if (n)
            reads.push_back({b.offset + 4 + pos, b.offset + 4 + pos + n, i});
    }
    std::sort(reads.begin(), reads.end(), [](auto &a, auto &b) { return a.begin < b.begin; });
    // Successful writes flush their output before publishing the index.
    // Reads can therefore use positioned I/O without touching stdio's cursor.
    auto read_region = [&](std::uint64_t at, void *data, std::size_t count) {
        if (file_read_exact_at(file_.get(), static_cast<file_offset_t>(at), data, count))
            throw io_error("Failed to read block bytes");
    };
#if (defined(__ANDROID__) || defined(SNFILETOOLS_TEST_PARALLEL_READS)) && !defined(_WIN32)
    // A run of one large read can be partitioned by byte range. Small reads
    // need many independent regions before thread startup can pay for itself.
    // Leave the ordinary serial merge and checkpoint path untouched otherwise.
    bool parallel_single = false, parallel_scattered = false;
    if (operation_checkpoint_hook == nullptr) {
        parallel_single = reads.size() == 1 && reads[0].end - reads[0].begin >= 8 * 1024 * 1024;
        if (reads.size() >= 4096) {
            std::uint64_t bytes = 0;
            parallel_scattered = true;
            for (std::size_t i = 0; i < reads.size(); ++i) {
                if (i && reads[i].begin <= reads[i - 1].end + read_merge_gap_bytes) {
                    parallel_scattered = false;
                    break;
                }
                bytes += reads[i].end - reads[i].begin;
            }
            parallel_scattered = parallel_scattered && bytes >= 256 * 1024;
        }
    }
    if (parallel_single || parallel_scattered) {
        // Capture the descriptor on the serialized caller. Workers use only
        // pread, never the shared FILE cursor, index, cache, or result states.
        const int fd = fileno(file_.get());
        if (fd < 0)
            throw io_error("Failed to obtain document descriptor");
        auto read_at = [fd](std::uint64_t offset, char *data, std::size_t size) {
            while (size) {
                const auto n = pread(fd, data, std::min(size, static_cast<std::size_t>(std::numeric_limits<ssize_t>::max())),
                                     static_cast<off_t>(offset));
                if (n < 0 && errno == EINTR)
                    continue;
                if (n <= 0) {
                    // Save errno in this worker before another call changes it.
                    const int error = n == 0 ? EIO : errno;
                    throw io_error("Failed to read block bytes", error);
                }
                data += n;
                offset += static_cast<std::size_t>(n);
                size -= static_cast<std::size_t>(n);
            }
        };
        constexpr std::size_t workers = 4;
        if (parallel_single) {
            auto &bytes = results[reads[0].index];
            bytes.resize(reads[0].end - reads[0].begin);
        } else {
            for (const auto &read : reads)
                results[read.index].resize(read.end - read.begin);
        }
        std::array<std::exception_ptr, workers> failures{};
        struct JoiningThreads {
            std::array<std::thread, workers> threads;
            std::size_t started = 0;
            void join() noexcept {
                for (std::size_t i = 0; i < started; ++i)
                    if (threads[i].joinable())
                        threads[i].join();
                started = 0;
            }
            ~JoiningThreads() { join(); }
        } joining;
        // thread_local hooks belong to the caller; explicitly capture only
        // the private failure seams. An operation checkpoint forces serial.
        const auto worker_hook = parallel_read_worker_hook;
        const auto spawn_hook = parallel_read_before_spawn_hook;
        for (std::size_t worker = 0; worker < workers; ++worker) {
            if (spawn_hook)
                spawn_hook(worker);
            joining.threads[worker] = std::thread([&, worker, worker_hook] {
                try {
                    if (parallel_single) {
                        auto &bytes = results[reads[0].index];
                        const auto begin = bytes.size() * worker / workers;
                        const auto end = bytes.size() * (worker + 1) / workers;
                        if (worker_hook)
                            worker_hook(worker, worker);
                        read_at(reads[0].begin + begin, bytes.data() + begin, end - begin);
                    } else {
                        const auto first = reads.size() * worker / workers;
                        const auto last = reads.size() * (worker + 1) / workers;
                        for (auto i = first; i < last; ++i) {
                            if (worker_hook)
                                worker_hook(worker, i);
                            const auto &read = reads[i];
                            auto &bytes = results[read.index];
                            read_at(read.begin, bytes.data(), bytes.size());
                        }
                    }
                } catch (...) {
                    failures[worker] = std::current_exception();
                }
            });
            ++joining.started;
        }
        joining.join();
        for (const auto &failure : failures)
            if (failure)
                std::rethrow_exception(failure);
        for (std::size_t i = 0; i < requests.size(); ++i) {
            requests[i].result->bytes = std::move(results[i]);
            requests[i].result->status = ReadState::ready;
        }
        return;
    }
#endif
    ByteBuffer workspace;
    for (std::size_t first = 0; first < reads.size();) {
        auto end = reads[first].end;
        auto last = first + 1;
        // A small bounded gap avoids another I/O call for nearby requests.
        // Endpoints are validated u32 file offsets; adding 64 cannot overflow.
        while (last < reads.size() && reads[last].begin <= end + read_merge_gap_bytes) {
            end = std::max(end, reads[last].end);
            ++last;
        }
        if (last == first + 1) {
            results[reads[first].index].resize(end - reads[first].begin);
            read_region(reads[first].begin, results[reads[first].index].data(), end - reads[first].begin);
            operation_checkpoint("read.region");
        } else {
            // Merged reads append chunks, so discard the old logical size but
            // retain the allocation. Singleton reads overwrite in place above.
            for (auto k = first; k < last; ++k)
                results[reads[k].index].clear();
            workspace.resize(std::min<std::uint64_t>(io_workspace_bytes, end - reads[first].begin));
            // Active intervals are removed as soon as their output is complete.
            std::vector<std::size_t> active;
            active.reserve(last - first);
            auto next = first;
            for (auto at = reads[first].begin; at < end;) {
                auto n = std::min<std::uint64_t>(workspace.size(), end - at);
                read_region(at, workspace.data(), n);
                operation_checkpoint("read.region");
                while (next < last && reads[next].begin < at + n)
                    active.push_back(next++);
                for (auto idx : active) {
                    auto &r = reads[idx];
                    auto begin = std::max(at, r.begin), finish = std::min(at + n, r.end);
                    if (finish > begin)
                        results[r.index].insert(results[r.index].end(), workspace.data() + begin - at,
                                                workspace.data() + finish - at);
                }
                at += n;
                std::erase_if(active, [&](auto idx) { return reads[idx].end <= at; });
            }
        }
        first = last;
    }
    for (std::size_t i = 0; i < requests.size(); ++i) {
        requests[i].result->bytes = std::move(results[i]);
        requests[i].result->status = ReadState::ready;
    }
}
void SNfile::execute_writes(const std::vector<WriteOperation> &requests,
                            const std::vector<CreateOperation> &creations,
                            const std::vector<AttachOperation> &attachments) {
    ensure_block_index();
    std::vector<BlockEdit> edits;
    edits.reserve(requests.size());
    for (const auto &r : requests) {
        const auto &b = block_index_->get(r.handle);
        auto pos = r.range ? BlockIndex::position(b, r.range->origin, r.range->offset, false) : 0;
        edits.push_back({r.handle, pos, r.range ? r.range->erase_bytes : b.size, r.bytes});
    }
    edits = block_index_->validate(std::move(edits));
    if (edits.empty() && creations.empty() && attachments.empty())
        return;
    if (!UndoJournal::configured())
        throw std::logic_error("Configure a persistent private journal directory before writing");
    bool direct = creations.empty() && attachments.empty() && block_index_->compact();
    for (const auto &e : edits) {
        auto kind = block_index_->get(e.handle).kind;
        direct = direct && e.erase == e.data.size() && (kind == BlockKind::opaque || kind == BlockKind::field);
    }
    std::unique_ptr<BlockIndex> candidate;
    BlockWritePlan plan;
    std::vector<BlockHandle> created_handles;
    created_handles.reserve(creations.size());
    if (direct) {
        plan.size = state_.size;
        for (const auto &e : edits)
            plan.spans.push_back(
                WriteSpan::borrowed_bytes(block_index_->get(e.handle).offset + 4 + e.position, e.data));
        plan.coalesce_literals(file_.get());
    } else {
        operation_checkpoint("plan.candidate");
        candidate = std::make_unique<BlockIndex>(*block_index_);
        plan = candidate->replace_many(*block_index_, edits, creations, attachments, created_handles, file_.get());
    }
    std::vector<char> workspace(plan.workspace_size(state_.size)); // before journal/document mutation
    if (!writable_) {
        File replacement(file_open_shared(data_path_, "r+b"), &file_close);
        file_state_t current{};
        if (!replacement)
            throw io_error("Open document for writing");
        if (file_state(replacement.get(), &current))
            throw io_error("Inspect document before writing");
        if (!same_state(current, state_))
            throw std::runtime_error("Document changed before opening for writing");
        // Buffer sequential relocation/output, while batch reads and journal
        // backups use positioned reads and avoid stdio read-ahead amplification.
        (void)std::setvbuf(replacement.get(), nullptr, _IOFBF, stream_buffer_bytes);
        file_ = std::move(replacement);
        writable_ = true;
    }
    std::unique_ptr<UndoJournal> journal;
    try {
        journal = std::make_unique<UndoJournal>(file_.get(), data_path_, plan, state_, workspace, journal_path_);
        write_failed_ = true;
        plan.execute(file_.get(), workspace, state_.size);
        journal->commit();
        if (file_state(file_.get(), &state_))
            throw io_error("Cannot inspect completed write");
        if (candidate)
            block_index_ = std::move(candidate);
        write_failed_ = false;
        // All storage for the result and handle vector was allocated before mutation.
        for (std::size_t i = 0; i < creations.size(); ++i) {
            creations[i].result->handle = created_handles[i];
            creations[i].result->status = CreatedState::ready;
        }
    } catch (const std::exception &error) {
        bool committed = journal && journal->committed();
        journal.reset(); // release lock and close journal before closing/recovering document
        if (file_) {
            file_.reset();
        }
        block_index_.reset();
        write_failed_ = true;
        const std::string message = error.what();
        try {
            // Includes ambiguous commit-flush failures whose close may have completed the marker.
            committed = committed || UndoJournal::committed_on_disk(journal_path_);
            UndoJournal::recover(data_path_, &journal_path_);
        } catch (const std::exception &recovery) {
            throw std::runtime_error(
                std::string(committed ? "Write committed; recovery failed: " : "Write failed; recovery required: ") +
                message + "; recovery: " + recovery.what());
        }
        throw std::runtime_error(std::string(committed ? "Write committed; later failure: "
                                                       : "Write not applied; original data restored: ") +
                                 message);
    }
}

SNfile::~SNfile() {
    cancel_pending();
}
} // namespace snfiletools::internal

snfiletools::internal::SNfile &open_file(const std::filesystem::path &path) {
    const auto normalized = normalize_document_path(path);
    auto file_path = normalized;
    auto extension = normalized.extension().native();
    ascii_to_lower(extension);
    auto data_path = normalized;
    if (extension == std::filesystem::path(".note").native()) {
        // The note path is both the cache identity and the data path.
    } else if (extension == std::filesystem::path(".mark").native()) {
        file_path.replace_extension();
        auto companion_extension = file_path.extension().native();
        ascii_to_lower(companion_extension);
        if (companion_extension != std::filesystem::path(".pdf").native())
            throw std::runtime_error("File type not supported");
    } else if (extension == std::filesystem::path(".pdf").native()) {
        data_path += ".mark";
    } else {
        throw std::runtime_error("File type not supported");
    }

    for (const auto &file : snfiletools::internal::open_snfiles) {
        if (file->get_path() == file_path) {
            auto *selected = file.get(); // touch() rotates the owning vector.
            selected->touch();
            selected->cancel_pending();
            selected->refresh_if_changed();
            return *selected;
        }
    }
    auto &files = snfiletools::internal::open_snfiles;
    // Reserve and open before eviction: allocation/open failure preserves the cache.
    files.reserve(snfiletools::internal::max_open_snfiles);
    auto replacement = std::make_unique<snfiletools::internal::SNfile>(std::move(file_path), std::move(data_path));
    if (files.size() >= snfiletools::internal::max_open_snfiles)
        files.pop_back();
    files.insert(files.begin(), std::move(replacement));
    return *files.front();
}

void set_journal_directory(const std::filesystem::path &path) {
    if (snfiletools::internal::UndoJournal::configure(path))
        snfiletools::internal::open_snfiles.clear(); // Rebuild cached journal locations and invalidate handles.
}
namespace snfiletools::internal {
ReadResult::ReadResult(std::shared_ptr<ReadState> state) : state_(std::move(state)) {
}
ByteBuffer &ReadResult::data() {
    if (!state_ || state_->status != ReadState::ready)
        throw std::logic_error("Read result is not ready (pending, failed, or cancelled)");
    return state_->bytes;
}
const ByteBuffer &ReadResult::data() const {
    return const_cast<ReadResult *>(this)->data();
}
} // namespace snfiletools::internal
