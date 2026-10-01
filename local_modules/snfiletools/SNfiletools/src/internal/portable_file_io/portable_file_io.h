#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t file_offset_t;
/* Seconds since the Unix epoch (1970-01-01 00:00:00 UTC). */
typedef int64_t file_timestamp_t;

FILE *file_open(const char *path, const char *mode);
/* Existing binary documents only (rb/r+b). Share read/write/delete on Windows;
 * callers use file_lock_exclusive for transactions. */
FILE *file_open_shared(const char *path, const char *mode);
int file_close(FILE *file);
int file_seek(FILE *file, file_offset_t offset, int origin);
/* Read exactly bytes at offset; return -1 on error or premature EOF (EIO).
 * Caller must flush pending stream output first and serialize stream access.
 * The stream position after this operation is unspecified. */
int file_read_exact_at(FILE *file, file_offset_t offset, void *buffer, size_t bytes);
/* Flush/discard stdio buffers before writing at an explicit position. The caller
 * must serialize access and flush after writes before publishing a transaction.
 * The stream position after this operation is unspecified. */
int file_write_exact_at(FILE *file, file_offset_t offset, const void *buffer, size_t bytes);
file_offset_t file_tell(FILE *file);
int file_size(FILE *file, file_offset_t *size);
int file_last_modified(FILE *file, file_timestamp_t *timestamp);
int file_is_regular(FILE *file);

typedef struct file_state {
  uint64_t device, identity, size;
  int64_t modified_seconds, modified_subseconds;
  int64_t changed_seconds, changed_subseconds;
} file_state_t;
int file_state(FILE *file, file_state_t *state);
int file_path_state(const char *path, file_state_t *state);
int file_truncate(FILE *file, file_offset_t size);
int file_sync(FILE *file);
int file_lock_exclusive(FILE *file);
int file_unlock(FILE *file);
// Synchronize directory entries; failure must not be treated as durable success.
int file_sync_directory(const char *path);

#ifdef __cplusplus
}

#include <filesystem>
#include <string>

inline FILE *file_open(const std::string& path, const char *mode) {
  return file_open(path.c_str(), mode);
}

inline FILE *file_open(const std::string& path, const std::string& mode) {
  return file_open(path.c_str(), mode.c_str());
}

inline FILE *file_open(const std::filesystem::path& path, const char *mode) {
  const std::u8string utf8 = path.u8string();

  return file_open(
      reinterpret_cast<const char*>(utf8.c_str()),
      mode
  );
}

inline FILE *file_open_shared(const std::filesystem::path &path, const char *mode) {
  const auto utf8 = path.u8string();
  return file_open_shared(reinterpret_cast<const char *>(utf8.c_str()), mode);
}
inline int file_path_state(const std::filesystem::path &path, file_state_t *state) {
#ifdef _WIN32
  const auto utf8 = path.u8string();
  return file_path_state(reinterpret_cast<const char *>(utf8.c_str()), state);
#else
  return file_path_state(path.c_str(), state);
#endif
}

#endif
