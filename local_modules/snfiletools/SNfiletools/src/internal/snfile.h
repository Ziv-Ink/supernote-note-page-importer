#pragma once

#include <snfiletools.h>

#include "batch_operations.h"
#include "portable_file_io/portable_file_io.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Select the cached object, cancel unfinished work, refresh, and promote it.
// The cache owns the result. Eviction or a changed journal directory invalidates
// references; MRU rotation does not. Refresh can invalidate block handles.
snfiletools::internal::SNfile &open_file(const std::filesystem::path &path);

namespace snfiletools::internal {
class BlockIndex;
enum class FileType {
    invalid,
    mark,
    note,
};

class SNfile final {
  public:
    // Called by open_file with an already normalized document identity.
    SNfile(std::filesystem::path path, std::filesystem::path data_path);

    SNfile(const SNfile &) = delete;
    SNfile &operator=(const SNfile &) = delete;
    SNfile(SNfile &&) = delete;
    SNfile &operator=(SNfile &&) = delete;

    [[nodiscard]] std::int64_t get_size() const;
    [[nodiscard]] const std::filesystem::path &get_path() const noexcept { return file_path_; }

    ~SNfile();

    [[nodiscard]] std::vector<BlockInfo> list_blocks();
    [[nodiscard]] BlockInfo get_block_info(BlockHandle handle);
    [[nodiscard]] std::vector<BlockInfo> get_children(BlockHandle handle);

    // Queuing owns input bytes and never accesses the filesystem.
    [[nodiscard]] ReadResult read(BlockHandle handle);
    [[nodiscard]] ReadResult read(BlockHandle handle, ReadRange range);
    void write(BlockHandle handle, std::span<const char> data);
    void write(BlockHandle handle, WriteRange range, std::span<const char> data);
    void write(BlockHandle handle, ByteBuffer &&data);
    void write(BlockHandle handle, WriteRange range, ByteBuffer &&data);
    [[nodiscard]] CreatedBlock create(BlockKind kind, std::span<const char> data);
    [[nodiscard]] CreatedBlock create(BlockKind kind, ByteBuffer &&data);
    void attach(BlockTarget parent, std::string key, BlockTarget target);
    // Consumes the queue on success or failure. Mixed reads/writes are rejected.
    void run();

  private:
    friend auto ::open_file(const std::filesystem::path &path) -> SNfile &;
    void cancel_pending() noexcept;
    ReadResult queue_read(BlockHandle handle, std::optional<ReadRange> range);
    void queue_write(BlockHandle handle, std::optional<WriteRange> range, ByteBuffer data);
    void execute_reads(const std::vector<ReadOperation> &requests);
    void execute_writes(const std::vector<WriteOperation> &requests, const std::vector<CreateOperation> &creations,
                        const std::vector<AttachOperation> &attachments);
    std::vector<ReadOperation> reads_;
    std::vector<WriteOperation> writes_;
    std::vector<CreateOperation> creations_;
    std::vector<AttachOperation> attachments_;

    // Public data operations call touch(); path lookup and parsing do not.
    void touch() const noexcept;

    // Check document state, recover when needed, and invalidate stale indexes.
    void refresh_if_changed();
    void ensure_block_index();

    FileType file_type_ = FileType::invalid;
    std::filesystem::path file_path_;
    std::filesystem::path data_path_;
    file_state_t state_{};
    std::filesystem::path journal_path_;
    std::unique_ptr<BlockIndex> block_index_;
    bool writable_ = false;
    bool write_failed_ = false;

    using File = std::unique_ptr<std::FILE, decltype(&file_close)>;
    File file_{nullptr, &file_close};
};

inline constexpr std::size_t max_open_snfiles = 10;
// Most recently used first. Eviction destroys the least recently used object.
inline std::vector<std::unique_ptr<SNfile>> open_snfiles;
} // namespace snfiletools::internal
