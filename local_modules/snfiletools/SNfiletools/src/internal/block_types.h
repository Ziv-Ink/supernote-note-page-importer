#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Private types shared by SNfile, the block index, and queued operations.
namespace snfiletools::internal {
class SNfile;
enum class BlockOrigin { begin, end };
enum class BlockKind { index, file, page, layer, keyword, title, link, opaque, trails, trail, field };

// Valid for this cached file object until eviction or an external file change.
// Writes preserve surviving handles; replacing a parent's entire payload retires
// all its descendant handles. Offsets returned in BlockInfo are snapshots.
struct BlockHandle {
    std::uint64_t generation = 0;
    std::uint32_t id = 0;
    bool operator==(const BlockHandle &) const = default;
};

struct BlockInfo {
    BlockHandle handle;
    BlockHandle parent; // generation == 0 for a top-level block
    std::string name;
    std::uint64_t offset; // length prefix, not payload
    std::uint32_t size;   // payload bytes
};

using ByteBuffer = std::vector<char>;
struct CreatedState {
    enum Status { pending, ready, failed, cancelled } status = pending;
    BlockHandle handle{};
};
class CreatedBlock {
  public:
    [[nodiscard]] BlockHandle handle() const;
  private:
    friend class SNfile;
    friend struct BlockTarget;
    explicit CreatedBlock(std::shared_ptr<CreatedState> state) : state_(std::move(state)) {}
    std::shared_ptr<CreatedState> state_;
};
// A target is either a committed handle or a creation from this same queue.
struct BlockTarget {
    BlockHandle existing{};
    std::shared_ptr<CreatedState> pending{};
    BlockTarget(BlockHandle handle) : existing(handle) {}
    BlockTarget(const CreatedBlock &created) : pending(created.state_) {}
};
struct ReadRange {
    BlockOrigin origin = BlockOrigin::begin;
    std::int64_t offset = 0;
    std::size_t bytes = 0;
};
struct WriteRange {
    BlockOrigin origin = BlockOrigin::begin;
    std::int64_t offset = 0;
    std::size_t erase_bytes = 0;
};
struct ReadState;
class ReadResult {
  public:
    [[nodiscard]] ByteBuffer &data();
    [[nodiscard]] const ByteBuffer &data() const;

  private:
    friend class SNfile;
    explicit ReadResult(std::shared_ptr<ReadState> state);
    std::shared_ptr<ReadState> state_;
};
} // namespace snfiletools::internal
