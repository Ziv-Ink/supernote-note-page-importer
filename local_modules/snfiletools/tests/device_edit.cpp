#include "../SNfiletools/src/internal/snfile.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace snfiletools_test {
namespace fs = std::filesystem;
using namespace snfiletools::internal;

namespace {
void progress(const char *stage) {
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "SNfiletoolsNative", "%s", stage);
#else
  (void)stage;
#endif
}
bool same_bytes(const fs::path &left, const fs::path &right) {
  if (fs::file_size(left) != fs::file_size(right)) return false;
  std::ifstream a(left, std::ios::binary), b(right, std::ios::binary);
  if (!a || !b) throw std::runtime_error("Cannot open byte-comparison inputs");
  std::array<char, 65536> x{}, y{};
  while (a.read(x.data(), x.size()) || a.gcount()) {
    b.read(y.data(), a.gcount());
    if (b.gcount() != a.gcount() ||
        !std::equal(x.begin(), x.begin() + a.gcount(), y.begin())) return false;
  }
  if (a.bad() || b.bad()) throw std::runtime_error("Byte comparison failed");
  return true;
}

// Stream the bytes explicitly: some device/FUSE paths reject the optimized
// filesystem copy after a note has been published with a fresh inode.
void copy_bytes(const fs::path &source, const fs::path &destination) {
  std::ifstream input(source, std::ios::binary);
  if (!input) throw std::runtime_error("Cannot read note snapshot source");
  std::ofstream output(destination, std::ios::binary | std::ios::trunc);
  if (!output) throw std::runtime_error("Cannot create note snapshot");
  std::array<char, 65536> buffer{};
  while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
    output.write(buffer.data(), input.gcount());
    if (!output) throw std::runtime_error("Cannot write note snapshot");
  }
  if (input.bad()) throw std::runtime_error("Cannot finish reading note snapshot");
  output.close();
  if (!output) throw std::runtime_error("Cannot finish writing note snapshot");
}

BlockInfo file_metadata(SNfile &file) {
  const auto blocks = file.list_blocks();
  const auto it = std::find_if(blocks.begin(), blocks.end(), [](const auto &b) {
    return b.name == "FILE_FEATURE" && b.parent.generation == 0;
  });
  if (it == blocks.end()) throw std::runtime_error("No FILE_FEATURE block");
  return *it;
}

BlockInfo named_top_level(SNfile &file, const std::string &name) {
  const auto blocks = file.list_blocks();
  const auto it = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
    return b.name == name && b.parent.generation == 0;
  });
  if (it == blocks.end()) throw std::runtime_error("Block not found: " + name);
  return *it;
}

BlockInfo referenced_block(SNfile &file, BlockHandle parent, const std::string &key) {
  auto read = file.read(parent);
  file.run();
  const std::string metadata(read.data().begin(), read.data().end());
  const std::string prefix = "<" + key + ":";
  const auto at = metadata.find(prefix);
  if (at == std::string::npos) throw std::runtime_error("Missing block reference: " + key);
  const auto begin = at + prefix.size();
  const auto end = metadata.find('>', begin);
  if (end == std::string::npos) throw std::runtime_error("Invalid block reference: " + key);
  std::uint64_t offset = 0;
  const auto parsed = std::from_chars(metadata.data() + begin, metadata.data() + end, offset);
  if (parsed.ec != std::errc{} || parsed.ptr != metadata.data() + end || offset == 0)
    throw std::runtime_error("Empty or invalid block reference: " + key);
  const auto blocks = file.list_blocks();
  const auto it = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
    return b.offset == offset && b.name == key;
  });
  if (it == blocks.end()) throw std::runtime_error("Reference target not indexed: " + key);
  return *it;
}

// A blank page can reference offset zero. It must not fall through to another
// page's strokes. Resolve the requested page before looking up its TOTALPATH.
ByteBuffer page_totalpath(SNfile &file, const std::string &page_key) {
  const auto page = named_top_level(file, page_key);
  auto read = file.read(page.handle);
  file.run();
  const std::string metadata(read.data().begin(), read.data().end());
  const std::string prefix = "<TOTALPATH:";
  const auto at = metadata.find(prefix);
  if (at == std::string::npos) throw std::runtime_error("Page has no TOTALPATH field");
  const auto begin = at + prefix.size();
  const auto end = metadata.find('>', begin);
  if (end == std::string::npos) throw std::runtime_error("Invalid TOTALPATH reference");
  std::uint64_t offset = 0;
  const auto parsed = std::from_chars(metadata.data() + begin, metadata.data() + end, offset);
  if (parsed.ec != std::errc{} || parsed.ptr != metadata.data() + end)
    throw std::runtime_error("Invalid TOTALPATH offset");
  if (offset == 0) return ByteBuffer(4, '\0');
  auto total = file.read(referenced_block(file, page.handle, "TOTALPATH").handle);
  file.run();
  return std::move(total.data());
}

BlockInfo page_bitmap(SNfile &file, const std::string &page_key) {
  const auto page = named_top_level(file, page_key);
  const auto layer = referenced_block(file, page.handle, "MAINLAYER");
  return referenced_block(file, layer.handle, "LAYERBITMAP");
}

ByteBuffer load_pattern(const fs::path &path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("Cannot open bitmap pattern");
  const auto size = fs::file_size(path);
  if (size == 0 || size > 16 * 1024 * 1024)
    throw std::invalid_argument("Bitmap pattern size is invalid");
  ByteBuffer bytes(static_cast<std::size_t>(size));
  if (!stream.read(bytes.data(), bytes.size())) throw std::runtime_error("Cannot read bitmap pattern");
  return bytes;
}
}

// A diagnostic, not a public library API. The caller saves the native note first
// and serializes this with other library operations. Keep both snapshots.
std::string edit_and_restore(const fs::path &path, const fs::path &backup) {
  if (path.extension() != ".note")
    throw std::invalid_argument("The edit test requires a .note file");
  const auto edited = fs::path(backup.string() + ".edited.note");
  if (fs::exists(backup) || fs::exists(edited))
    throw std::runtime_error("Test snapshot paths must be new");
  auto &file = open_file(path);
  const auto metadata = file_metadata(file);
  fs::create_directories(backup.parent_path());
  copy_bytes(path, backup);
  if (!same_bytes(path, backup)) throw std::runtime_error("Backup verification failed");

  const auto restore = [&] {
    // Block writes can compact save history. Undoing only the added tag would
    // not restore the original whole file, so restore the complete snapshot.
    copy_bytes(backup, path);
    if (!same_bytes(path, backup))
      throw std::runtime_error("Restore failed; retained backup: " + backup.string());
    (void)open_file(path).list_blocks();
  };
  const std::string marker = "<SNFILETOOLS_TEST:temporary-byte-edit>";
  std::uint64_t marker_offset = 0;
  std::uintmax_t edited_size = 0;
  try {
    file.write(metadata.handle, {BlockOrigin::end, 0, 0},
               std::span<const char>(marker.data(), marker.size()));
    file.run();
    const auto after = file_metadata(file);
    auto read = file.read(after.handle, {BlockOrigin::end,
        -static_cast<std::int64_t>(marker.size()), marker.size()});
    file.run();
    if (std::string(read.data().begin(), read.data().end()) != marker)
      throw std::runtime_error("Library read-back did not match the edit");
    marker_offset = after.offset + 4 + after.size - marker.size();
    std::ifstream disk(path, std::ios::binary);
    disk.seekg(static_cast<std::streamoff>(marker_offset));
    std::string actual(marker.size(), '\0');
    if (!disk.read(actual.data(), actual.size()) || actual != marker)
      throw std::runtime_error("On-disk bytes did not match the edit");
    if (same_bytes(path, backup)) throw std::runtime_error("The file did not change");
    copy_bytes(path, edited);
    if (!same_bytes(path, edited)) throw std::runtime_error("Edited snapshot verification failed");
    edited_size = fs::file_size(path);
  } catch (...) {
    // This catch is required cleanup for a test that mutates a real open note.
    restore();
    throw;
  }
  restore();
  return "PASS: appended " + std::to_string(marker.size()) +
      " metadata bytes at offset " + std::to_string(marker_offset) +
      "; verified library and disk read-back; original=" +
      std::to_string(fs::file_size(backup)) + " bytes, edited=" +
      std::to_string(edited_size) + " bytes; restored every original byte";
}

void restore_note(const fs::path &path, const fs::path &backup) {
  if (path.extension() != ".note" || fs::equivalent(path, backup))
    throw std::invalid_argument("Restore requires a separate note backup");
  copy_bytes(backup, path);
  if (!same_bytes(path, backup)) throw std::runtime_error("Byte-exact restore failed");
  (void)open_file(path).list_blocks();
}

// Leave the edit in place so the native reader and its file watcher can be
// observed. The JS test restores the snapshot in finally, including SDK errors.
std::string edit_totalpath(const fs::path &path, const fs::path &backup) {
  if (path.extension() != ".note")
    throw std::invalid_argument("TOTALPATH test requires a .note file");
  const fs::path edited = backup.string() + ".edited.note";
  if (fs::exists(backup) || fs::exists(edited))
    throw std::runtime_error("Test snapshot paths must be new");
  auto &file = open_file(path);
  const auto blocks = file.list_blocks();
  const auto total = std::find_if(blocks.begin(), blocks.end(), [](const auto &b) {
    return b.name == "TOTALPATH" && b.size > 4;
  });
  if (total == blocks.end())
    throw std::runtime_error("No nonempty TOTALPATH: draw a stroke in the throwaway note first");
  const auto previous_size = total->size;
  fs::create_directories(backup.parent_path());
  copy_bytes(path, backup);
  if (!same_bytes(path, backup)) throw std::runtime_error("Backup verification failed");
  try {
    // A structurally valid empty trails block: little-endian uint32 count=0.
    // Replace the entire TOTALPATH through the library's write planner.
    const std::array<char, 4> empty_trails{};
    file.write(total->handle, std::span<const char>(empty_trails));
    file.run();
    const auto after = file.get_block_info(total->handle);
    auto read = file.read(total->handle);
    file.run();
    if (read.data() != ByteBuffer(4, '\0') || !file.get_children(total->handle).empty())
      throw std::runtime_error("TOTALPATH read-back failed");
    std::ifstream disk(path, std::ios::binary);
    disk.seekg(static_cast<std::streamoff>(after.offset + 4));
    std::array<char, 4> actual{};
    if (!disk.read(actual.data(), actual.size()) || actual != empty_trails)
      throw std::runtime_error("TOTALPATH disk read-back failed");
    if (same_bytes(path, backup)) throw std::runtime_error("TOTALPATH edit did not change the file");
    copy_bytes(path, edited);
    return "TOTALPATH edit applied: " + std::to_string(previous_size) +
        " -> 4 payload bytes; cleared trails; verified library and disk read-back";
  } catch (...) {
    restore_note(path, backup);
    throw;
  }
}

std::string transfer_totalpath(const fs::path &path, const fs::path &source,
                           const fs::path &backup, const std::string &page_key, bool append,
                           const std::string &source_page_key = "") {
  progress("TOTALPATH transfer entered");
  if (path.extension() != ".note" || source.extension() != ".note" ||
      fs::equivalent(path, source))
    throw std::invalid_argument("Paste requires two different .note files");
  const fs::path edited = backup.string() + ".edited.note";
  if (fs::exists(backup) || fs::exists(edited))
    throw std::runtime_error("Test snapshot paths must be new");

  progress("Opening source");
  auto &input = open_file(source);
  progress("Indexing source");
  ByteBuffer source_bytes;
  if (!source_page_key.empty()) {
    source_bytes = page_totalpath(input, source_page_key);
  } else {
    const auto source_blocks = input.list_blocks();
    const auto source_total = std::find_if(source_blocks.begin(), source_blocks.end(), [](const auto &b) {
      return b.name == "TOTALPATH" && b.size > 4;
    });
    if (source_total == source_blocks.end())
      throw std::runtime_error("Source note has no nonempty TOTALPATH");
    auto source_read = input.read(source_total->handle);
    input.run();
    source_bytes = std::move(source_read.data());
  }
  progress("Source TOTALPATH read complete");
  const auto count = [](const ByteBuffer &bytes) -> std::uint32_t {
    if (bytes.size() < 4) throw std::runtime_error("Truncated TOTALPATH count");
    std::uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i)
      value |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i])) << (8 * i);
    return value;
  };
  const auto added = count(source_bytes);
  progress("Opening destination");
  auto &output = open_file(path);
  progress("Destination opened; indexing blocks");
  const auto blocks = output.list_blocks();
  progress("Destination indexed; finding page");
  const auto page = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
    return b.name == page_key && b.parent.generation == 0;
  });
  if (page == blocks.end()) throw std::runtime_error("Destination page not found: " + page_key);
  progress("Destination page found; reading metadata");
  auto page_read = output.read(page->handle);
  output.run();
  progress("Destination page metadata read");
  const std::string page_metadata(page_read.data().begin(), page_read.data().end());
  const std::string key = "<TOTALPATH:";
  const auto key_position = page_metadata.find(key);
  if (key_position == std::string::npos) throw std::runtime_error("Page has no TOTALPATH field");
  const auto begin = key_position + key.size();
  const auto end = page_metadata.find('>', begin);
  if (end == std::string::npos) throw std::runtime_error("Invalid TOTALPATH field");
  std::uint64_t total_offset = 0;
  const auto parsed = std::from_chars(page_metadata.data() + begin,
                                      page_metadata.data() + end, total_offset);
  if (parsed.ec != std::errc{} || parsed.ptr != page_metadata.data() + end)
    throw std::runtime_error("Invalid TOTALPATH offset");
  const auto total = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
    return b.name == "TOTALPATH" && b.offset == total_offset;
  });
  if (total_offset != 0 && total == blocks.end())
    throw std::runtime_error("Page TOTALPATH target was not indexed");
  progress("Destination TOTALPATH reference parsed");
  ByteBuffer merged(4, '\0');
  if (total != blocks.end() && total->size != 0) {
    auto before = output.read(total->handle);
    output.run();
    merged = std::move(before.data());
  }
  progress("Destination TOTALPATH read complete");
  const auto previous = count(merged);
  const auto combined = static_cast<std::uint64_t>(append ? previous : 0) + added;
  if (combined > std::numeric_limits<std::uint32_t>::max() ||
      merged.size() + source_bytes.size() - 4 > std::numeric_limits<std::uint32_t>::max())
    throw std::length_error("Merged TOTALPATH exceeds format limits");
  if (append) merged.insert(merged.end(), source_bytes.begin() + 4, source_bytes.end());
  else merged = source_bytes;
  for (unsigned i = 0; i < 4; ++i) merged[i] = static_cast<char>(combined >> (8 * i));

  progress("Creating verified backup");
  fs::create_directories(backup.parent_path());
  copy_bytes(path, backup);
  if (!same_bytes(path, backup)) throw std::runtime_error("Backup verification failed");
  try {
    progress("Writing destination TOTALPATH");
    BlockHandle handle;
    if (total != blocks.end()) {
      handle = total->handle;
      output.write(handle, std::span<const char>(merged));
      output.run();
    } else {
      auto created = output.create(BlockKind::trails, std::span<const char>(merged));
      output.attach(page->handle, "TOTALPATH", created);
      output.run();
      handle = created.handle();
    }
    auto after = output.read(handle);
    output.run();
    if (after.data() != merged || output.get_children(handle).size() != combined)
      throw std::runtime_error("Pasted TOTALPATH read-back failed");
    const auto info = output.get_block_info(handle);
    std::ifstream disk(path, std::ios::binary);
    disk.seekg(static_cast<std::streamoff>(info.offset + 4));
    ByteBuffer actual(merged.size());
    if (!disk.read(actual.data(), actual.size()) || actual != merged)
      throw std::runtime_error("Pasted TOTALPATH disk verification failed");
    if (append && same_bytes(path, backup)) throw std::runtime_error("Paste did not change the file");
    copy_bytes(path, edited);
    return std::string(append ? "TOTALPATH paste applied to " : "TOTALPATH copy applied to ") + page_key + ": " + std::to_string(append ? previous : 0) +
        " existing + " + std::to_string(added) + " imported = " +
        std::to_string(combined) + " trails; " + std::to_string(merged.size()) +
        (append ? " payload bytes; original and imported trail bytes preserved; library and disk verified" : " payload bytes; source payload copied byte-for-byte; library and disk verified");
  } catch (...) {
    restore_note(path, backup);
    throw;
  }
}

std::string paste_totalpath(const fs::path &path, const fs::path &source,
                           const fs::path &backup, const std::string &page_key) {
  return transfer_totalpath(path, source, backup, page_key, true);
}

std::string copy_totalpath(const fs::path &path, const fs::path &source,
                          const fs::path &backup, const std::string &page_key) {
  return transfer_totalpath(path, source, backup, page_key, false);
}

std::string copy_page_totalpath(const fs::path &path, const fs::path &source,
                               const fs::path &backup, const std::string &page_key,
                               const std::string &source_page_key) {
  if (source_page_key.empty()) throw std::invalid_argument("Source page is required");
  return transfer_totalpath(path, source, backup, page_key, false, source_page_key);
}

bool bitmap_unchanged(const fs::path &path, const fs::path &reference,
                      const std::string &page_key) {
  auto &original = open_file(reference);
  auto before = original.read(page_bitmap(original, page_key).handle);
  original.run();
  const auto expected = std::move(before.data());
  auto &current = open_file(path);
  auto after = current.read(page_bitmap(current, page_key).handle);
  current.run();
  return after.data() == expected;
}

// Compare payloads, not file offsets or whole-file bytes: native saves can move
// blocks while preserving the imported TOTALPATH. Read both through the library.
bool totalpath_unchanged(const fs::path &path, const fs::path &reference,
                         const std::string &page_key) {
  auto &original = open_file(reference);
  const auto expected = page_totalpath(original, page_key);
  auto &current = open_file(path);
  return page_totalpath(current, page_key) == expected;
}

bool bitmap_matches(const fs::path &path, const fs::path &pattern, const std::string &page_key) {
  const auto expected = load_pattern(pattern);
  auto &file = open_file(path);
  const auto bitmap = page_bitmap(file, page_key);
  auto read = file.read(bitmap.handle);
  file.run();
  return read.data() == expected;
}

// Notes caches working data by inode. Publish the verified edited snapshot
// at the same path with a fresh inode before asking the SDK to reopen it.
void reopen_snapshot(const fs::path &path, const fs::path &snapshot) {
  if (path.extension() != ".note" || fs::equivalent(path, snapshot))
    throw std::invalid_argument("Reopening requires separate note and snapshot files");
  if (!same_bytes(path, snapshot))
    throw std::runtime_error("Current note changed since the edited snapshot; refusing replacement");
  const fs::path temporary = path.string() + ".snfiletools-reopen";
  if (fs::exists(temporary)) throw std::runtime_error("Reopen temporary file already exists");
  struct Cleanup {
    fs::path path;
    ~Cleanup() { std::error_code error; fs::remove(path, error); }
  } cleanup{temporary};
  copy_bytes(snapshot, temporary);
  if (!same_bytes(temporary, snapshot)) throw std::runtime_error("Reopen copy verification failed");
  fs::rename(temporary, path);
  if (!same_bytes(path, snapshot)) throw std::runtime_error("Reopened note verification failed");
}

std::string replace_bitmap(const fs::path &path, const fs::path &pattern,
                           const fs::path &backup, const std::string &page_key) {
  if (path.extension() != ".note") throw std::invalid_argument("Bitmap test requires a .note");
  const fs::path edited = backup.string() + ".edited.note";
  if (fs::exists(backup) || fs::exists(edited))
    throw std::runtime_error("Test snapshot paths must be new");
  const auto bytes = load_pattern(pattern);
  auto &file = open_file(path);
  const auto bitmap = page_bitmap(file, page_key);
  const auto old_size = bitmap.size;
  fs::create_directories(backup.parent_path());
  copy_bytes(path, backup);
  if (!same_bytes(path, backup)) throw std::runtime_error("Backup verification failed");
  try {
    file.write(bitmap.handle, std::span<const char>(bytes));
    file.run();
    auto read = file.read(bitmap.handle);
    file.run();
    if (read.data() != bytes) throw std::runtime_error("Bitmap library read-back failed");
    const auto after = file.get_block_info(bitmap.handle);
    std::ifstream disk(path, std::ios::binary);
    disk.seekg(static_cast<std::streamoff>(after.offset + 4));
    ByteBuffer actual(bytes.size());
    if (!disk.read(actual.data(), actual.size()) || actual != bytes)
      throw std::runtime_error("Bitmap disk read-back failed");
    copy_bytes(path, edited);
    return "BITMAP replacement applied to " + page_key + ".MAINLAYER.LAYERBITMAP: " +
        std::to_string(old_size) + " -> " + std::to_string(bytes.size()) +
        " payload bytes; verified library and disk read-back";
  } catch (...) {
    restore_note(path, backup);
    throw;
  }
}
}
