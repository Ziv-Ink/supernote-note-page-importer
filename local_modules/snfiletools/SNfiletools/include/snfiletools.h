#pragma once
#include <cstdint>
#include <filesystem>

// The caller must serialize all library access.
// Reports the note/mark size, including when the input path ends in .pdf.
std::int64_t get_size(const std::filesystem::path &path);

// Configure persistent app-private storage before the first write.
void set_journal_directory(const std::filesystem::path &path);
