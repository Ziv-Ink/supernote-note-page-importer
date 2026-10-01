#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../SNfiletools/src/internal/snfile.h"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <future>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace supernote_feature_Snfiletools {
std::int64_t getSize(std::string path);
void setJournalDirectory(std::string path);
std::string testMetadataEditAndRestore(std::string path, std::string backupPath);
std::string testTotalPathEdit(std::string path, std::string backupPath);
void restoreTestNote(std::string path, std::string backupPath);
std::string testTotalPathPaste(std::string path, std::string sourcePath,
                             std::string backupPath, std::string pageKey);
std::string testBitmapReplace(std::string path, std::string patternPath,
                            std::string backupPath, std::string pageKey);
bool bitmapMatches(std::string path, std::string patternPath, std::string pageKey);
void testReopenSnapshot(std::string path, std::string snapshotPath);
std::string testTotalPathCopy(std::string path, std::string sourcePath,
                            std::string backupPath, std::string pageKey);
std::string copyPageTotalPath(std::string path, std::string sourcePath,
                            std::string backupPath, std::string pageKey,
                            std::string sourcePageKey);
bool testBitmapUnchanged(std::string path, std::string referencePath, std::string pageKey);
bool testTotalPathUnchanged(std::string path, std::string referencePath, std::string pageKey);
}

namespace {
namespace fs = std::filesystem;
using supernote_feature_Snfiletools::getSize;
using supernote_feature_Snfiletools::setJournalDirectory;

std::vector<fs::path> fixtures(const char *folder, const char *extension) {
  std::vector<fs::path> paths;
  for (const auto &entry : fs::directory_iterator(fs::path(SN_TEST_FILES_DIR) / folder)) {
    if (entry.is_regular_file() && entry.path().extension() == extension)
      paths.push_back(entry.path());
  }
  REQUIRE_FALSE(paths.empty());
  return paths;
}
}

TEST_CASE("Bridge reports every note and PDF/mark size") {
  for (const auto &path : fixtures("notes", ".note")) {
    INFO(path.string());
    CHECK(getSize(path.string()) == fs::file_size(path));
    CHECK(getSize((path.parent_path() / "." / path.filename()).string()) == fs::file_size(path));
  }
  for (const auto &pdf : fixtures("pdf_marks", ".pdf")) {
    INFO(pdf.string());
    const fs::path mark = pdf.string() + ".mark";
    REQUIRE(fs::is_regular_file(mark));
    CHECK(getSize(pdf.string()) == fs::file_size(mark));
    CHECK(getSize(mark.string()) == fs::file_size(mark));
  }
}

TEST_CASE("Bridge propagates invalid path failures and remains usable") {
  CHECK_THROWS_AS(getSize(""), std::runtime_error);
  CHECK_THROWS_AS(getSize("unsupported.txt"), std::runtime_error);
  const auto missing = fs::path(SN_TEST_FILES_DIR) / "__missing__.note";
  REQUIRE_FALSE(fs::exists(missing));
  CHECK_THROWS_AS(getSize(missing.string()), std::runtime_error);
  const auto path = fixtures("notes", ".note").front();
  CHECK(getSize(path.string()) == fs::file_size(path));
}

TEST_CASE("Bridge serializes concurrent reads across cache eviction") {
  auto paths = fixtures("notes", ".note");
  const auto pdfs = fixtures("pdf_marks", ".pdf");
  paths.insert(paths.end(), pdfs.begin(), pdfs.end());
  REQUIRE(paths.size() > 10);
  std::vector<std::future<bool>> readers;
  for (int worker = 0; worker < 4; ++worker) {
    readers.push_back(std::async(std::launch::async, [paths] {
      for (int pass = 0; pass < 3; ++pass) {
        for (const auto &path : paths) {
          const fs::path data = path.extension() == ".pdf" ? fs::path(path.string() + ".mark") : path;
          if (getSize(path.string()) != static_cast<std::int64_t>(fs::file_size(data)))
            return false;
        }
      }
      return true;
    }));
  }
  for (auto &reader : readers)
    CHECK(reader.get());
}

TEST_CASE("Journal configuration propagates errors and preserves readable files") {
  CHECK_THROWS_AS(setJournalDirectory(""), std::invalid_argument);
  const auto path = fixtures("notes", ".note").front();
  CHECK_THROWS(setJournalDirectory(path.string()));
  // CTest runs in a private build directory; no supplied fixtures are changed.
  const auto journal = fs::current_path() / "test-journal";
  setJournalDirectory(journal.string());
  CHECK(fs::is_directory(journal));
  setJournalDirectory(journal.string());
  CHECK(getSize(path.string()) == fs::file_size(path));
}

TEST_CASE("Metadata edit changes real note copies and restores every byte") {
  const auto root = fs::current_path() / "edit-tests";
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const auto contents = [](const fs::path &path) {
    std::ifstream stream(path, std::ios::binary);
    REQUIRE(stream.is_open());
    return std::string(std::istreambuf_iterator<char>(stream), {});
  };
  for (const auto &source : fixtures("notes", ".note")) {
    INFO(source.string());
    const auto note = root / source.filename();
    const auto backup = root / (source.stem().string() + ".backup.note");
    const auto edited = fs::path(backup.string() + ".edited.note");
    fs::copy_file(source, note, fs::copy_options::overwrite_existing);
    fs::remove(backup);
    fs::remove(edited);
    const auto original = contents(source);
    const auto report = supernote_feature_Snfiletools::testMetadataEditAndRestore(
        note.string(), backup.string());
    CHECK(report.starts_with("PASS:"));
    CHECK(contents(backup) == original);
    CHECK(contents(note) == original);
    CHECK(contents(source) == original);
    const auto changed = contents(edited);
    CHECK(changed != original);
    CHECK(changed.find("<SNFILETOOLS_TEST:temporary-byte-edit>") != std::string::npos);
    CHECK(getSize(note.string()) == original.size());
    CHECK_THROWS(supernote_feature_Snfiletools::testMetadataEditAndRestore(
        note.string(), backup.string()));
    CHECK(contents(note) == original);
  }
  CHECK_THROWS_AS(supernote_feature_Snfiletools::testMetadataEditAndRestore(
      "file.pdf", (root / "invalid.backup.note").string()), std::invalid_argument);
  CHECK_THROWS(supernote_feature_Snfiletools::testMetadataEditAndRestore(
      (root / "missing.note").string(), (root / "missing.backup.note").string()));
  CHECK_FALSE(fs::exists(root / "missing.backup.note"));
}

TEST_CASE("TOTALPATH edit leaves changed bytes for reader observation and restores exactly") {
  const auto root = fs::current_path() / "totalpath-tests";
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const auto contents = [](const fs::path &path) {
    std::ifstream stream(path, std::ios::binary);
    REQUIRE(stream.is_open());
    return std::string(std::istreambuf_iterator<char>(stream), {});
  };
  std::size_t edited_count = 0;
  for (const auto &source : fixtures("notes", ".note")) {
    INFO(source.string());
    const auto note = root / source.filename();
    const auto backup = root / (source.stem().string() + ".backup.note");
    const auto edited = fs::path(backup.string() + ".edited.note");
    fs::copy_file(source, note, fs::copy_options::overwrite_existing);
    fs::remove(backup);
    fs::remove(edited);
    const auto original = contents(source);
    if (source.filename() == "blank.note") {
      CHECK_THROWS_WITH(supernote_feature_Snfiletools::testTotalPathEdit(
          note.string(), backup.string()),
          "No nonempty TOTALPATH: draw a stroke in the throwaway note first");
      CHECK(contents(note) == original);
      CHECK_FALSE(fs::exists(backup));
      continue;
    }
    const auto report = supernote_feature_Snfiletools::testTotalPathEdit(
        note.string(), backup.string());
    CHECK(report.starts_with("TOTALPATH edit applied:"));
    CHECK(contents(note) != original);
    CHECK(contents(note) == contents(edited));
    CHECK(contents(backup) == original);
    supernote_feature_Snfiletools::restoreTestNote(note.string(), backup.string());
    CHECK(contents(note) == original);
    CHECK(contents(source) == original);
    ++edited_count;
  }
  CHECK(edited_count > 0);
}

TEST_CASE("TOTALPATH paste imports every nonblank note into a blank page and preserves source") {
  const auto root = fs::current_path() / "paste-tests";
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const auto blank = fs::path(SN_TEST_FILES_DIR) / "notes/blank.note";
  REQUIRE(fs::is_regular_file(blank));
  const auto contents = [](const fs::path &path) {
    std::ifstream stream(path, std::ios::binary);
    REQUIRE(stream.is_open());
    return std::string(std::istreambuf_iterator<char>(stream), {});
  };
  for (const auto &source : fixtures("notes", ".note")) {
    if (source.filename() == "blank.note") continue;
    INFO(source.string());
    const auto note = root / source.filename();
    const auto backup = root / (source.stem().string() + ".backup.note");
    const auto edited = fs::path(backup.string() + ".edited.note");
    fs::copy_file(blank, note, fs::copy_options::overwrite_existing);
    fs::remove(backup);
    fs::remove(edited);
    const auto source_before = contents(source);
    const auto destination_before = contents(note);
    const auto report = supernote_feature_Snfiletools::testTotalPathPaste(
        note.string(), source.string(), backup.string(), "PAGE1");
    CHECK(report.starts_with("TOTALPATH paste applied to PAGE1:"));
    CHECK(contents(note) != destination_before);
    CHECK(contents(backup) == destination_before);
    CHECK(contents(edited) == contents(note));
    CHECK(contents(source) == source_before);
    // Paste again onto the populated page to exercise merging existing trails.
    const auto second = root / (source.stem().string() + ".second.note");
    fs::remove(second);
    fs::remove(fs::path(second.string() + ".edited.note"));
    const auto first_paste = contents(note);
    CHECK_NOTHROW(supernote_feature_Snfiletools::testTotalPathPaste(
        note.string(), source.string(), second.string(), "PAGE1"));
    CHECK(contents(note) != first_paste);
    CHECK(contents(source) == source_before);
    supernote_feature_Snfiletools::restoreTestNote(note.string(), backup.string());
    CHECK(contents(note) == destination_before);
    CHECK_THROWS(supernote_feature_Snfiletools::testTotalPathPaste(
        note.string(), source.string(), backup.string(), "PAGE1"));
    CHECK(contents(note) == destination_before);
  }
  const auto note = root / "rejection.note";
  fs::copy_file(blank, note, fs::copy_options::overwrite_existing);
  const auto backup = root / "rejection.backup.note";
  CHECK_THROWS_AS(supernote_feature_Snfiletools::testTotalPathPaste(
      note.string(), note.string(), backup.string(), "PAGE1"), std::invalid_argument);
  CHECK_THROWS_WITH(supernote_feature_Snfiletools::testTotalPathPaste(
      note.string(), blank.string(), backup.string(), "PAGE1"),
      "Source note has no nonempty TOTALPATH");
  CHECK_FALSE(fs::exists(backup));
}

TEST_CASE("Main-layer bitmap replacement verifies the real pattern on every note and restores exactly") {
  const auto root = fs::current_path() / "bitmap-tests";
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const std::string pattern = SN_BITMAP_TEST_PATTERN;
  REQUIRE(fs::file_size(pattern) == 94324);
  const auto contents = [](const fs::path &path) {
    std::ifstream stream(path, std::ios::binary);
    REQUIRE(stream.is_open());
    return std::string(std::istreambuf_iterator<char>(stream), {});
  };
  for (const auto &source : fixtures("notes", ".note")) {
    INFO(source.string());
    const auto note = root / source.filename();
    const auto backup = root / (source.stem().string() + ".backup.note");
    fs::copy_file(source, note, fs::copy_options::overwrite_existing);
    fs::remove(backup);
    fs::remove(fs::path(backup.string() + ".edited.note"));
    const auto original = contents(source);
    REQUIRE_FALSE(supernote_feature_Snfiletools::bitmapMatches(note.string(), pattern, "PAGE1"));
    CHECK_THROWS(supernote_feature_Snfiletools::testBitmapReplace(
        note.string(), "missing.rle", backup.string(), "PAGE1"));
    CHECK_THROWS(supernote_feature_Snfiletools::testBitmapReplace(
        note.string(), pattern, backup.string(), "PAGE99999"));
    CHECK_FALSE(fs::exists(backup));
    const auto report = supernote_feature_Snfiletools::testBitmapReplace(
        note.string(), pattern, backup.string(), "PAGE1");
    CHECK(report.starts_with("BITMAP replacement applied to PAGE1.MAINLAYER.LAYERBITMAP:"));
    CHECK(supernote_feature_Snfiletools::bitmapMatches(note.string(), pattern, "PAGE1"));
    CHECK(contents(backup) == original);
    CHECK(contents(note) != original);
    CHECK(contents(source) == original);
    CHECK_THROWS(supernote_feature_Snfiletools::testReopenSnapshot(note.string(), backup.string()));
    CHECK_THROWS(supernote_feature_Snfiletools::testReopenSnapshot(note.string(), note.string()));
    const auto edited = contents(note);
    supernote_feature_Snfiletools::testReopenSnapshot(note.string(), backup.string() + ".edited.note");
    CHECK(contents(note) == edited);
    CHECK(supernote_feature_Snfiletools::bitmapMatches(note.string(), pattern, "PAGE1"));
    CHECK_THROWS(supernote_feature_Snfiletools::testBitmapReplace(
        note.string(), pattern, backup.string(), "PAGE1"));
    supernote_feature_Snfiletools::restoreTestNote(note.string(), backup.string());
    CHECK(contents(note) == original);
    CHECK_FALSE(supernote_feature_Snfiletools::bitmapMatches(note.string(), pattern, "PAGE1"));
  }
}

TEST_CASE("Raw TOTALPATH copy preserves bitmap, replaces existing trails, and keeps backups") {
  const auto root = fs::current_path() / "copy-tests";
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const auto blank = fs::path(SN_TEST_FILES_DIR) / "notes/blank.note";
  for (const auto &source : fixtures("notes", ".note")) {
    if (source == blank) continue;
    INFO(source.string());
    const auto note = root / source.filename();
    const auto backup = root / (source.stem().string() + ".backup.note");
    fs::copy_file(blank, note, fs::copy_options::overwrite_existing);
    fs::remove(backup);
    fs::remove(backup.string() + ".edited.note");
    CHECK(supernote_feature_Snfiletools::testTotalPathCopy(
        note.string(), source.string(), backup.string(), "PAGE1").starts_with("TOTALPATH copy applied to PAGE1:"));
    CHECK(supernote_feature_Snfiletools::testBitmapUnchanged(note.string(), backup.string(), "PAGE1"));
    CHECK(supernote_feature_Snfiletools::testTotalPathUnchanged(
        note.string(), backup.string() + ".edited.note", "PAGE1"));
    const auto altered = root / (source.stem().string() + ".altered.note");
    fs::remove(altered);
    fs::remove(altered.string() + ".edited.note");
    CHECK_NOTHROW(supernote_feature_Snfiletools::testTotalPathEdit(note.string(), altered.string()));
    CHECK_FALSE(supernote_feature_Snfiletools::testTotalPathUnchanged(
        note.string(), backup.string() + ".edited.note", "PAGE1"));
    supernote_feature_Snfiletools::restoreTestNote(note.string(), altered.string());
    CHECK(supernote_feature_Snfiletools::testTotalPathUnchanged(
        note.string(), backup.string() + ".edited.note", "PAGE1"));
    // Copy over the already populated destination: the helper checks exact source
    // payload on disk and parses its expected trail count, rather than appending.
    const auto second = root / (source.stem().string() + ".second.note");
    fs::remove(second);
    fs::remove(second.string() + ".edited.note");
    CHECK(supernote_feature_Snfiletools::testTotalPathCopy(
        note.string(), source.string(), second.string(), "PAGE1").find("0 existing +") != std::string::npos);
    CHECK(supernote_feature_Snfiletools::testBitmapUnchanged(note.string(), backup.string(), "PAGE1"));
    supernote_feature_Snfiletools::restoreTestNote(note.string(), backup.string());
  }
}

TEST_CASE("Explicit page copy preserves sibling pages and uses blank source PAGE1") {
  using namespace snfiletools::internal;
  using supernote_feature_Snfiletools::copyPageTotalPath;
  const auto root = fs::current_path() / "explicit-page-tests";
  fs::remove_all(root);
  fs::create_directories(root);
  setJournalDirectory((root / "journals").string());
  const auto samples = fs::path(SN_TEST_FILES_DIR) / "notes";
  const auto source = root / "source.note";
  const auto dest = root / "destination.note";
  fs::copy_file(samples / "N007_Trip_Log.note", source);
  fs::copy_file(samples / "N002_Project_Followup.note", dest);
  const auto contents = [](const fs::path &p) {
    std::ifstream stream(p, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), {});
  };
  const auto payload = [](const fs::path &p, const std::string &pageKey) {
    auto &file = open_file(p);
    const auto blocks = file.list_blocks();
    const auto page = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
      return b.name == pageKey && b.parent.generation == 0;
    });
    REQUIRE(page != blocks.end());
    auto read = file.read(page->handle);
    file.run();
    const std::string metadata(read.data().begin(), read.data().end());
    const auto begin = metadata.find("<TOTALPATH:") + 11;
    const auto end = metadata.find('>', begin);
    std::uint64_t offset = 0;
    REQUIRE(std::from_chars(metadata.data() + begin, metadata.data() + end, offset).ec == std::errc{});
    if (!offset) return ByteBuffer(4, '\0');
    const auto total = std::find_if(blocks.begin(), blocks.end(), [&](const auto &b) {
      return b.name == "TOTALPATH" && b.offset == offset;
    });
    REQUIRE(total != blocks.end());
    auto totalRead = file.read(total->handle);
    file.run();
    return ByteBuffer(totalRead.data());
  };
  const auto sourceOriginal = contents(source);
  const auto original = contents(dest);
  const auto backup = root / "first.note";
  CHECK_NOTHROW(copyPageTotalPath(dest.string(), source.string(), backup.string(), "PAGE2", "PAGE1"));
  CHECK(payload(dest, "PAGE2") == payload(source, "PAGE1"));
  CHECK(supernote_feature_Snfiletools::testTotalPathUnchanged(dest.string(), backup.string(), "PAGE1"));
  CHECK(supernote_feature_Snfiletools::testBitmapUnchanged(dest.string(), backup.string(), "PAGE1"));
  CHECK(supernote_feature_Snfiletools::testBitmapUnchanged(dest.string(), backup.string(), "PAGE2"));
  CHECK(contents(source) == sourceOriginal);
  CHECK(contents(backup) == original);
  const auto beforeInvalid = contents(dest);
  CHECK_THROWS(copyPageTotalPath(dest.string(), source.string(), (root / "invalid.note").string(), "PAGE2", "PAGE99"));
  CHECK(contents(dest) == beforeInvalid);
  CHECK_FALSE(fs::exists(root / "invalid.note"));
  CHECK_THROWS(copyPageTotalPath(dest.string(), source.string(), (root / "invalid-dest.note").string(), "PAGE99", "PAGE1"));
  CHECK(contents(dest) == beforeInvalid);
  CHECK_FALSE(fs::exists(root / "invalid-dest.note"));

  // Source page one is blank but page two still contains strokes. The copy
  // must clear the destination rather than finding later nonempty TOTALPATH.
  REQUIRE(payload(source, "PAGE2").size() > 4);
  CHECK_NOTHROW(copyPageTotalPath(source.string(), (samples / "blank.note").string(),
      (root / "clear-source.note").string(), "PAGE1", "PAGE1"));
  REQUIRE(payload(source, "PAGE1") == ByteBuffer(4, '\0'));
  REQUIRE(payload(source, "PAGE2").size() > 4);
  CHECK_NOTHROW(copyPageTotalPath(dest.string(), source.string(),
      (root / "blank-copy.note").string(), "PAGE2", "PAGE1"));
  CHECK(payload(dest, "PAGE2") == ByteBuffer(4, '\0'));
  CHECK(supernote_feature_Snfiletools::testTotalPathUnchanged(dest.string(), backup.string(), "PAGE1"));
}
