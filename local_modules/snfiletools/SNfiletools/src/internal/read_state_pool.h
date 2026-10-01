#pragma once
#include <array>
#include <mutex>
#include "batch_operations.h"

namespace snfiletools::internal {
// Recycle storage, never payloads: every successful read overwrites its output.
// Only the last result owner returns a state. The shared pool also outlives
// results released after file eviction or static teardown.
class ReadStatePool {
  public:
    static std::shared_ptr<ReadState> acquire() {
        static const auto pool = std::shared_ptr<ReadStatePool>(new ReadStatePool);
        auto *state = pool->take();
        state->status = ReadState::pending;
        // shared_ptr calls this deleter even if allocating its control block fails.
        return {state, [owner = pool](ReadState *p) noexcept { owner->release(p); }};
    }

  private:
    static constexpr std::size_t max_bytes = 32 * 1024 * 1024;
    static constexpr std::size_t max_states = 4096;
    std::mutex mutex_;
    std::array<std::unique_ptr<ReadState>, max_states> free_{};
    std::size_t count_ = 0;
    std::size_t bytes_ = 0;

    ReadState *take() {
        {
            const std::lock_guard lock(mutex_);
            if (count_) {
                auto *state = free_[--count_].release();
                bytes_ -= state->bytes.capacity();
                return state;
            }
        }
        return new ReadState;
    }

    void release(ReadState *state) noexcept {
        std::unique_ptr<ReadState> owned(state);
        try {
            const std::lock_guard lock(mutex_);
            if (count_ < max_states && state->bytes.capacity() <= max_bytes - bytes_) {
                bytes_ += state->bytes.capacity();
                // Fixed slots make returning a result allocation-free.
                free_[count_++] = std::move(owned);
            }
        } catch (...) {
            // A failed mutex acquisition must not throw from result destruction.
            // The local owner simply frees the state instead of recycling it.
        }
    }
};
} // namespace snfiletools::internal
