#pragma once
#include "block_types.h"
#include <optional>
namespace snfiletools::internal {
struct ReadState {
    enum Status { pending, ready, failed, cancelled };
    Status status = pending;
    ByteBuffer bytes;
};
struct ReadOperation {
    BlockHandle handle;
    std::optional<ReadRange> range;
    std::shared_ptr<ReadState> result;
};
struct WriteOperation {
    BlockHandle handle;
    std::optional<WriteRange> range;
    ByteBuffer bytes;
};
struct CreateOperation {
    BlockKind kind;
    ByteBuffer bytes;
    std::shared_ptr<CreatedState> result;
};
struct AttachOperation {
    BlockTarget parent;
    std::string key;
    BlockTarget target;
};
} // namespace snfiletools::internal
