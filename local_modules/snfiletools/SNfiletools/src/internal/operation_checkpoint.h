#pragma once
#include <string_view>

namespace snfiletools::internal {
// Internal observation/fault-injection seam, null in normal use. Tests observe
// read regions and planning, inject failures, or terminate child processes at
// transaction boundaries and individual writes to verify recovery.
// Historical *.synced checkpoint names denote fflush, not fsync.
inline thread_local void (*operation_checkpoint_hook)(std::string_view) = nullptr;
inline void operation_checkpoint(std::string_view stage) {
    if (operation_checkpoint_hook != nullptr)
        operation_checkpoint_hook(stage);
}
} // namespace snfiletools::internal
