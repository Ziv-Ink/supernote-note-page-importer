#pragma once
#include "portable_file_io/portable_file_io.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <system_error>
#include <tuple>

namespace snfiletools::internal {
inline constexpr std::size_t io_workspace_bytes = 256 * 1024;
inline constexpr std::size_t stream_buffer_bytes = 64 * 1024;
inline constexpr std::size_t read_merge_gap_bytes = 64;
inline constexpr std::size_t positioned_write_threshold = 4096;

inline bool same_state(const file_state_t &a, const file_state_t &b) {
    return std::tie(a.device, a.identity, a.size, a.modified_seconds, a.modified_subseconds, a.changed_seconds,
                    a.changed_subseconds) == std::tie(b.device, b.identity, b.size, b.modified_seconds,
                                                      b.modified_subseconds, b.changed_seconds, b.changed_subseconds);
}
// Capture errno at the failure site, before cleanup can replace it.
inline std::system_error io_error(const char *operation, int error = errno) {
    return std::system_error(error ? error : EIO, std::generic_category(), operation);
}
inline void block_read_exact(std::FILE *file, std::uint64_t offset, void *data, std::size_t bytes) {
    if (bytes == 0)
        return;
    if (file_seek(file, static_cast<file_offset_t>(offset), SEEK_SET) != 0)
        throw io_error("Seek block for reading");
    if (std::fread(data, 1, bytes, file) != bytes)
        throw io_error("Read block bytes", std::ferror(file) ? errno : EIO);
}

} // namespace snfiletools::internal
