#include "block_write_plan.h"
#include "operation_checkpoint.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace snfiletools::internal {
namespace {
void write_exact(std::FILE *file, std::uint64_t offset, const void *bytes, std::size_t count) {
    if (count == 0)
        return;
    if (file_seek(file, static_cast<file_offset_t>(offset), SEEK_SET) != 0)
        throw io_error("Seek block for writing");
    if (std::fwrite(bytes, 1, count, file) != count)
        throw io_error("Write block bytes");
}
} // namespace

void BlockWritePlan::coalesce_literals(std::FILE *file) {
    // Dense tiny edits benefit from fewer reads, journal records and writes.
    // Cap both amplification and total temporary storage; sparse edits and
    // already contiguous ranges keep their existing buffered output path.
    constexpr std::uint64_t max_gap = 64;
    constexpr std::size_t max_group = 256 * 1024, max_storage = 1024 * 1024;
    if (spans.size() < 4)
        return;
    std::size_t storage = 0, output = 0;
    for (std::size_t first = 0; first < spans.size();) {
        auto last = first + 1;
        const auto begin = spans[first].destination;
        auto end = begin + spans[first].size;
        auto edited = spans[first].size;
        if (!spans[first].copy && edited <= max_group) {
            while (last < spans.size()) {
                const auto &next = spans[last];
                if (next.copy || next.destination < end || next.destination - end > max_gap ||
                    next.size > max_group || next.destination - begin > max_group - next.size)
                    break;
                end = next.destination + next.size;
                edited += next.size;
                ++last;
            }
        }
        const auto merged_size = end - begin;
        if (last - first >= 4 && merged_size > edited && merged_size <= edited * 32 &&
            merged_size <= max_storage - storage) {
            std::string joined(static_cast<std::size_t>(merged_size), '\0');
            if (file_read_exact_at(file, static_cast<file_offset_t>(begin), joined.data(), joined.size()))
                throw io_error("Read coalesced write gaps");
            for (auto i = first; i < last; ++i) {
                const auto bytes = spans[i].bytes();
                if (!bytes.empty())
                    std::memcpy(joined.data() + spans[i].destination - begin, bytes.data(), bytes.size());
            }
            // The journal will back up the whole merged range, including gaps,
            // before execute() can modify any document bytes.
            spans[output++] = WriteSpan::owned_bytes(begin, std::move(joined));
            storage += static_cast<std::size_t>(merged_size);
        } else {
            for (auto i = first; i < last; ++i, ++output)
                if (output != i)
                    spans[output] = std::move(spans[i]);
        }
        first = last;
    }
    spans.erase(spans.begin() + output, spans.end());
}

std::size_t BlockWritePlan::workspace_size(std::uint64_t original_size) const {
    std::uint64_t largest = size < original_size ? original_size - size : 0, end = 0, contiguous = 0;
    for (const auto &s : spans)
        if (!s.copy || s.source != s.destination) {
            if (s.destination == end)
                contiguous += s.size;
            else
                contiguous = s.size;
            end = s.destination + s.size;
            largest = std::max(largest, contiguous);
        }
    return static_cast<std::size_t>(std::min<std::uint64_t>(largest, io_workspace_bytes));
}
void BlockWritePlan::execute(std::FILE *file, std::size_t buffer_bytes) const {
    if (!buffer_bytes)
        throw std::invalid_argument("Empty relocation buffer");
    file_offset_t original = 0;
    if (file_size(file, &original))
        throw io_error("Cannot inspect write target");
    std::vector<char> buffer(std::min(buffer_bytes, workspace_size(original)));
    execute(file, buffer, original);
}
void BlockWritePlan::execute(std::FILE *file, std::span<char> buffer, std::uint64_t original_size) const {
#ifndef _WIN32
    // Measured on large batches only. Relocation retains its ordered stdio path;
    // these literal-only plans have no source/destination dependencies.
    if (spans.size() >= positioned_write_threshold && size == original_size &&
        std::all_of(spans.begin(), spans.end(), [](const auto &s) { return !s.copy; })) {
        std::uint64_t start = 0;
        std::size_t used = 0;
        auto output = [&](std::uint64_t at, const char *data, std::size_t bytes) {
            if (file_write_exact_at(file, static_cast<file_offset_t>(at), data, bytes))
                throw io_error("Cannot write replacement range");
            operation_checkpoint("document.chunk");
        };
        auto flush = [&] {
            if (used) {
                output(start, buffer.data(), used);
                used = 0;
            }
        };
        for (const auto &span : spans) {
            const auto data = span.bytes();
            if (used && (span.destination != start + used || data.size() > buffer.size() - used))
                flush();
            if (data.size() > buffer.size() || buffer.empty())
                output(span.destination, data.data(), data.size());
            else {
                if (!used)
                    start = span.destination;
                std::memcpy(buffer.data() + used, data.data(), data.size());
                used += data.size();
            }
        }
        flush();
        operation_checkpoint("document.truncated");
        if (std::fflush(file))
            throw io_error("Failed to flush block write");
        operation_checkpoint("document.synced");
        return;
    }
#endif
    auto move = [&](const WriteSpan &span, bool backward) {
        if (buffer.empty())
            throw std::invalid_argument("Empty relocation workspace");
        std::uint64_t done = 0;
        while (done < span.size) {
            const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), span.size - done));
            const auto relative = backward ? span.size - done - count : done;
            block_read_exact(file, span.source + relative, buffer.data(), count);
            write_exact(file, span.destination + relative, buffer.data(), count);
            operation_checkpoint("document.chunk");
            done += count;
        }
    };
    // Copy spans preserve order. A left-moving destination cannot overwrite a
    // right-moving span's source (and vice versa); perform each group in memmove
    // order. Literal writes wait until ALL original source bytes are consumed.
    for (const auto &span : spans)
        if (span.copy && span.destination < span.source)
            move(span, false);
    for (auto it = spans.rbegin(); it != spans.rend(); ++it)
        if (it->copy && it->destination > it->source)
            move(*it, true);
    // Seek only between nonadjacent literal ranges; contiguous fwrite calls
    // share stdio buffering instead of flushing once per queued edit.
    std::uint64_t literal_end = std::numeric_limits<std::uint64_t>::max();
    for (const auto &span : spans)
        if (!span.copy) {
            const auto data = span.bytes();
            if (span.destination != literal_end &&
                file_seek(file, static_cast<file_offset_t>(span.destination), SEEK_SET))
                throw io_error("Cannot seek replacement range");
            if (std::fwrite(data.data(), 1, data.size(), file) != data.size())
                throw io_error("Cannot write replacement range");
            literal_end = span.destination + data.size();
            operation_checkpoint("document.chunk");
        }
    if (size != original_size && file_truncate(file, static_cast<file_offset_t>(size)) != 0)
        throw io_error("Failed to finalize block write; file may be partially changed");
    operation_checkpoint("document.truncated");
    if (std::fflush(file) != 0)
        throw io_error("Failed to flush block write");
    operation_checkpoint("document.synced");
}
} // namespace snfiletools::internal
