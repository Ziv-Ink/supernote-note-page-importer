#pragma once
#include "file_support.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace snfiletools::internal {
// A plan borrows queued request bytes until synchronous run() completes.
// Copy spans refer to the original document; owned literals hold generated metadata.
// Copying/moving a plan does not extend the lifetime of its borrowed request bytes.
struct WriteSpan {
    std::uint64_t source = 0, destination = 0, size = 0;
    std::string literal;
    bool copy = false;
    std::span<const char> borrowed{};
    static WriteSpan copy_from(std::uint64_t source, std::uint64_t destination, std::uint64_t size) {
        return WriteSpan(source, destination, size, {}, true, {});
    }
    static WriteSpan owned_bytes(std::uint64_t destination, std::string bytes) {
        const auto size = bytes.size();
        return WriteSpan(0, destination, size, std::move(bytes), false, {});
    }
    static WriteSpan borrowed_bytes(std::uint64_t destination, std::span<const char> bytes) {
        return WriteSpan(0, destination, bytes.size(), {}, false, bytes);
    }
    [[nodiscard]] std::span<const char> bytes() const {
        return borrowed.empty() ? std::span<const char>(literal) : borrowed;
    }

  private:
    WriteSpan(std::uint64_t from, std::uint64_t to, std::uint64_t count, std::string owned, bool is_copy,
              std::span<const char> borrowed_bytes)
        : source(from), destination(to), size(count), literal(std::move(owned)), copy(is_copy),
          borrowed(borrowed_bytes) {}
};
struct BlockWritePlan {
    std::vector<WriteSpan> spans;
    std::uint64_t size = 0;
    // Ordered, validated same-size literal edits only; runs before journal creation.
    void coalesce_literals(std::FILE *file);
    void execute(std::FILE *file, std::size_t buffer_bytes = io_workspace_bytes) const;
    void execute(std::FILE *file, std::span<char> buffer, std::uint64_t original_size) const;
    [[nodiscard]] std::size_t workspace_size(std::uint64_t original_size) const;
};

} // namespace snfiletools::internal
