#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <snfiletools.h>

namespace snfiletools_test {
std::string edit_and_restore(const std::filesystem::path &path,
                            const std::filesystem::path &backup);
std::string edit_totalpath(const std::filesystem::path &path,
                          const std::filesystem::path &backup);
void restore_note(const std::filesystem::path &path,
                  const std::filesystem::path &backup);
std::string paste_totalpath(const std::filesystem::path &path,
                           const std::filesystem::path &source,
                           const std::filesystem::path &backup,
                           const std::string &pageKey);
std::string copy_totalpath(const std::filesystem::path &path,
                          const std::filesystem::path &source,
                          const std::filesystem::path &backup,
                          const std::string &pageKey);
std::string copy_page_totalpath(const std::filesystem::path &path,
                               const std::filesystem::path &source,
                               const std::filesystem::path &backup,
                               const std::string &pageKey,
                               const std::string &sourcePageKey);
bool bitmap_unchanged(const std::filesystem::path &path,
                      const std::filesystem::path &reference,
                      const std::string &pageKey);
bool totalpath_unchanged(const std::filesystem::path &path,
                         const std::filesystem::path &reference,
                         const std::string &pageKey);
std::string replace_bitmap(const std::filesystem::path &path,
                           const std::filesystem::path &pattern,
                           const std::filesystem::path &backup,
                           const std::string &pageKey);
bool bitmap_matches(const std::filesystem::path &path,
                    const std::filesystem::path &pattern,
                    const std::string &pageKey);
void reopen_snapshot(const std::filesystem::path &path,
                     const std::filesystem::path &snapshot);
}

namespace supernote_feature_Snfiletools {

namespace {
// The library's document cache and journal configuration share process state.
std::mutex libraryMutex;
}  // namespace

// @SupernotePluginExport
std::string greet(std::string name) {
  return "Hello, " + name;
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::int64_t getSize(std::string path) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return ::get_size(std::filesystem::path(path));
}

// @SupernotePluginExport
// @SupernotePluginAsync
void setJournalDirectory(std::string path) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  ::set_journal_directory(std::filesystem::path(path));
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string testMetadataEditAndRestore(std::string path, std::string backupPath) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::edit_and_restore(path, backupPath);
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string testTotalPathEdit(std::string path, std::string backupPath) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::edit_totalpath(path, backupPath);
}

// @SupernotePluginExport
// @SupernotePluginAsync
void restoreTestNote(std::string path, std::string backupPath) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  snfiletools_test::restore_note(path, backupPath);
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string testTotalPathPaste(std::string path, std::string sourcePath,
                              std::string backupPath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::paste_totalpath(path, sourcePath, backupPath, pageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string testBitmapReplace(std::string path, std::string patternPath,
                             std::string backupPath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::replace_bitmap(path, patternPath, backupPath, pageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
bool bitmapMatches(std::string path, std::string patternPath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::bitmap_matches(path, patternPath, pageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
void testReopenSnapshot(std::string path, std::string snapshotPath) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  snfiletools_test::reopen_snapshot(path, snapshotPath);
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string testTotalPathCopy(std::string path, std::string sourcePath,
                             std::string backupPath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::copy_totalpath(path, sourcePath, backupPath, pageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
std::string copyPageTotalPath(std::string path, std::string sourcePath,
                             std::string backupPath, std::string pageKey,
                             std::string sourcePageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::copy_page_totalpath(path, sourcePath, backupPath,
                                              pageKey, sourcePageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
bool testBitmapUnchanged(std::string path, std::string referencePath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::bitmap_unchanged(path, referencePath, pageKey);
}

// @SupernotePluginExport
// @SupernotePluginAsync
bool testTotalPathUnchanged(std::string path, std::string referencePath, std::string pageKey) {
  const std::lock_guard<std::mutex> lock(libraryMutex);
  return snfiletools_test::totalpath_unchanged(path, referencePath, pageKey);
}

}  // namespace supernote_feature_Snfiletools
