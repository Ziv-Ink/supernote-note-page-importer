#pragma once

#include <cstddef>

namespace snfiletools::internal {
// Private fault-injection seams. The caller's hooks are captured before worker
// creation; worker hooks run in a worker and spawn hooks on the calling thread.
inline thread_local void (*parallel_read_worker_hook)(std::size_t worker, std::size_t job) = nullptr;
inline thread_local void (*parallel_read_before_spawn_hook)(std::size_t worker) = nullptr;
} // namespace snfiletools::internal
