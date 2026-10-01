#pragma once

#include "block_types.h"
#include "batch_operations.h"
#include <cstdio>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace snfiletools::internal {
inline constexpr std::uint32_t no_block = std::numeric_limits<std::uint32_t>::max();

struct BlockReference {
    std::size_t begin, end; // decimal value in cached metadata
    std::uint32_t target = no_block;
    std::string key;
};
struct IndexedBlock {
    std::uint64_t offset = 0;
    std::uint32_t size = 0;
    std::uint32_t parent = no_block;
    BlockKind kind = BlockKind::opaque;
    std::string name;
    std::string metadata{};
    std::vector<BlockReference> references{};
    bool live = true;
    std::uint64_t token = 0; // New identity when a retired nested slot is reused.
};

struct BlockEdit {
    BlockHandle handle;
    std::uint64_t position;
    std::size_t erase;
    std::span<const char> data;
};
struct BlockWritePlan;

class BlockIndex {
  public:
    explicit BlockIndex(std::FILE *file);
    [[nodiscard]] std::vector<BlockInfo> describe() const;
    [[nodiscard]] BlockInfo describe(BlockHandle handle) const;
    [[nodiscard]] std::vector<BlockInfo> children(BlockHandle handle) const;
    [[nodiscard]] std::vector<BlockEdit> validate(std::vector<BlockEdit> edits) const;
    [[nodiscard]] BlockWritePlan replace_many(const BlockIndex &original, const std::vector<BlockEdit> &edits,
                                              const std::vector<CreateOperation> &creations,
                                              const std::vector<AttachOperation> &attachments,
                                              std::vector<BlockHandle> &created_handles, std::FILE *file);
    [[nodiscard]] bool compact() const { return compact_; }
    [[nodiscard]] const IndexedBlock &get(BlockHandle handle) const;
    [[nodiscard]] static std::uint64_t position(const IndexedBlock &block, BlockOrigin origin, std::int64_t offset,
                                                bool reading);

  private:
    class ReplacementPlanner;
    void rebuild_traversal();
    std::vector<std::vector<std::uint32_t>> children_;
    std::vector<std::uint32_t> physical_, traversal_, owner_;
    bool compact_ = false;
    std::uint64_t generation_ = 0;
    std::uint32_t root_ = no_block;
    std::vector<IndexedBlock> blocks_;
};

} // namespace snfiletools::internal
