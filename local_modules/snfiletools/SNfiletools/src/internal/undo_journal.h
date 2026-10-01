#pragma once
#include "portable_file_io/portable_file_io.h"
#include <cstdio>
#include <filesystem>
#include <memory>
#include <span>

namespace snfiletools::internal {
struct BlockWritePlan;
class UndoJournal {
  public:
    UndoJournal(std::FILE *document, const std::filesystem::path &path, const BlockWritePlan &plan,
                const file_state_t &expected, std::span<char> workspace = {},
                const std::filesystem::path &cached_journal = {});
    ~UndoJournal();
    UndoJournal(const UndoJournal &) = delete;
    UndoJournal &operator=(const UndoJournal &) = delete;
    void commit();
    [[nodiscard]] bool committed() const noexcept;
    static bool configure(const std::filesystem::path &directory);
    static bool configured();
    static bool committed_on_disk(const std::filesystem::path &active);
    // Returns true when an existing journal was processed. Validates the entire
    // prepared journal before restoring a byte. Throws on corruption/identity mismatch.
    static bool recover(const std::filesystem::path &path, std::filesystem::path *resolved_journal = nullptr);
    static std::filesystem::path journal_path(const std::filesystem::path &path);

  private:
    static bool recover_one(const std::filesystem::path &path, const std::filesystem::path &active);
    struct State;
    std::unique_ptr<State> state_;
};
} // namespace snfiletools::internal
