#include "portable_file_io.h"
#include <cerrno>
#include <cstdint>
#include <limits>
#include <new>
#include <string>
#include <algorithm>

#ifdef _WIN32

#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <cstring>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

namespace {
int windows_error_to_errno(const DWORD error) {
  switch (error) {
  case ERROR_FILE_NOT_FOUND:
  case ERROR_PATH_NOT_FOUND:
    return ENOENT;
  case ERROR_ACCESS_DENIED:
  case ERROR_SHARING_VIOLATION:
  case ERROR_LOCK_VIOLATION:
    return EACCES;
  case ERROR_FILE_EXISTS:
  case ERROR_ALREADY_EXISTS:
    return EEXIST;
  case ERROR_INVALID_PARAMETER:
  case ERROR_INVALID_NAME:
    return EINVAL;
  case ERROR_NO_UNICODE_TRANSLATION:
    return EILSEQ;
  case ERROR_NOT_ENOUGH_MEMORY:
  case ERROR_OUTOFMEMORY:
    return ENOMEM;
  default:
    return EIO;
  }
}

bool utf8_to_utf16(const char *input, std::wstring *output) {
  const int input_length =
      MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1, nullptr, 0);
  if (input_length == 0) {
    errno = windows_error_to_errno(GetLastError());
    return false;
  }

  output->resize(static_cast<std::size_t>(input_length));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, -1,
                          output->data(), input_length) == 0) {
    errno = windows_error_to_errno(GetLastError());
    return false;
  }

  output->resize(static_cast<std::size_t>(input_length - 1));
  return true;
}
} // namespace

extern "C" FILE *file_open(const char *path, const char *mode) {
  if (path == nullptr || mode == nullptr) {
    errno = EINVAL;
    return nullptr;
  }

  try {
    std::wstring wide_path;
    std::wstring wide_mode;
    if (!utf8_to_utf16(path, &wide_path) || !utf8_to_utf16(mode, &wide_mode)) {
      return nullptr;
    }
    FILE *file = nullptr;
    const errno_t result =
        _wfopen_s(&file, wide_path.c_str(), wide_mode.c_str());
    if (result != 0) {
      errno = result;
      return nullptr;
    }
    return file;
  } catch (const std::bad_alloc &) {
    errno = ENOMEM;
    return nullptr;
  } catch (...) {
    errno = EIO;
    return nullptr;
  }
}

extern "C" FILE *file_open_shared(const char *path, const char *mode) {
  if (path == nullptr || mode == nullptr ||
      (std::strcmp(mode, "rb") != 0 && std::strcmp(mode, "r+b") != 0)) {
    errno = EINVAL;
    return nullptr;
  }
  try {
    std::wstring wide;
    if (!utf8_to_utf16(path, &wide)) return nullptr;
    const bool writable = std::strcmp(mode, "r+b") == 0;
    const HANDLE handle = CreateFileW(wide.c_str(), GENERIC_READ | (writable ? GENERIC_WRITE : 0),
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
      errno = windows_error_to_errno(GetLastError());
      return nullptr;
    }
    const int descriptor = _open_osfhandle(reinterpret_cast<intptr_t>(handle),
        _O_BINARY | (writable ? _O_RDWR : _O_RDONLY));
    if (descriptor == -1) {
      const int error = errno;
      CloseHandle(handle);
      errno = error;
      return nullptr;
    }
    FILE *stream = _fdopen(descriptor, mode);
    if (stream == nullptr) {
      const int error = errno;
      _close(descriptor);
      errno = error;
    }
    return stream;
  } catch (const std::bad_alloc &) {
    errno = ENOMEM;
    return nullptr;
  } catch (...) {
    errno = EIO;
    return nullptr;
  }
}

extern "C" int file_close(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return EOF;
  }
  return fclose(file);
}

extern "C" int file_seek(FILE *file, const file_offset_t offset,
                         const int origin) {
  if (file == nullptr ||
      (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END)) {
    errno = EINVAL;
    return -1;
  }
  return _fseeki64(file, offset, origin);
}

extern "C" file_offset_t file_tell(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return static_cast<file_offset_t>(-1);
  }

  const __int64 offset = _ftelli64(file);
  if (offset == static_cast<__int64>(-1)) {
    return static_cast<file_offset_t>(-1);
  }
  if (offset >
      static_cast<__int64>(std::numeric_limits<file_offset_t>::max())) {
    errno = EOVERFLOW;
    return static_cast<file_offset_t>(-1);
  }
  return static_cast<file_offset_t>(offset);
}

extern "C" int file_size(FILE *file, file_offset_t *size) {
  if (file == nullptr || size == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = _fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct _stat64 metadata{};
  if (_fstat64(descriptor, &metadata) != 0) {
    return -1;
  }
  if (metadata.st_size < 0 ||
      metadata.st_size >
          static_cast<__int64>(std::numeric_limits<file_offset_t>::max())) {
    errno = EOVERFLOW;
    return -1;
  }

  *size = static_cast<file_offset_t>(metadata.st_size);
  return 0;
}

extern "C" int file_last_modified(FILE *file, file_timestamp_t *timestamp) {
  if (file == nullptr || timestamp == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = _fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct _stat64 metadata{};
  if (_fstat64(descriptor, &metadata) != 0) {
    return -1;
  }

  *timestamp = static_cast<file_timestamp_t>(metadata.st_mtime);
  return 0;
}

extern "C" int file_is_regular(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = _fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct _stat64 metadata{};
  if (_fstat64(descriptor, &metadata) != 0) {
    return -1;
  }

  return (metadata.st_mode & _S_IFMT) == _S_IFREG ? 1 : 0;
}

namespace {
int state_from_handle(HANDLE handle, file_state_t *state) {
  BY_HANDLE_FILE_INFORMATION info{};
  if (!GetFileInformationByHandle(handle, &info)) { errno = EIO; return -1; }
  state->device = info.dwVolumeSerialNumber;
  state->identity = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
  state->size = (static_cast<uint64_t>(info.nFileSizeHigh) << 32) | info.nFileSizeLow;
  state->modified_seconds = info.ftLastWriteTime.dwHighDateTime;
  state->modified_subseconds = info.ftLastWriteTime.dwLowDateTime;
  state->changed_seconds = info.ftCreationTime.dwHighDateTime;
  state->changed_subseconds = info.ftCreationTime.dwLowDateTime;
  return 0;
}

} // namespace
extern "C" int file_state(FILE *file, file_state_t *state) {
  if (file == nullptr || state == nullptr) { errno = EINVAL; return -1; }
  return state_from_handle(reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file))), state);
}
extern "C" int file_path_state(const char *path, file_state_t *state) {
  if (path == nullptr || state == nullptr) { errno = EINVAL; return -1; }
  try {
    std::wstring wide;
    if (!utf8_to_utf16(path, &wide)) return -1;
    // Attribute access is independent of an existing CRT stream's read/write
    // sharing mode, and never reads bytes under a document's exclusive lock.
    const HANDLE handle = CreateFileW(wide.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle == INVALID_HANDLE_VALUE) { errno = windows_error_to_errno(GetLastError()); return -1; }
    const int result = state_from_handle(handle, state);
    const int saved_errno = errno;
    const bool closed = CloseHandle(handle) != 0;
    if (result != 0) { errno = saved_errno; return result; }
    if (!closed) { errno = EIO; return -1; }
    return 0;
  } catch (const std::bad_alloc &) { errno = ENOMEM; return -1; }
    catch (...) { errno = EIO; return -1; }
}

extern "C" int file_truncate(FILE *file, const file_offset_t size) {
  if (file == nullptr || size < 0) { errno = EINVAL; return -1; }
  if (fflush(file) != 0) return -1;
  const errno_t error = _chsize_s(_fileno(file), size);
  if (error != 0) { errno = static_cast<int>(error); return -1; }
  return 0;
}

extern "C" int file_sync(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  if (fflush(file) != 0) return -1;
  return _commit(_fileno(file));
}

extern "C" int file_lock_exclusive(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  OVERLAPPED overlapped{};
  const auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
  if (!LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
                  0, MAXDWORD, MAXDWORD, &overlapped)) { errno = EACCES; return -1; }
  return 0;
}

extern "C" int file_unlock(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  OVERLAPPED overlapped{};
  const auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
  if (!UnlockFileEx(handle, 0, MAXDWORD, MAXDWORD, &overlapped)) { errno = EIO; return -1; }
  return 0;
}

extern "C" int file_sync_directory(const char *path) {
  if (path == nullptr) { errno = EINVAL; return -1; }
  try {
    std::wstring wide;
    if (!utf8_to_utf16(path, &wide)) return -1;
    const HANDLE handle = CreateFileW(wide.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle == INVALID_HANDLE_VALUE) { errno = EACCES; return -1; }
    const bool flushed = FlushFileBuffers(handle) != 0;
    CloseHandle(handle);
    if (!flushed) { errno = EIO; return -1; }
    return 0;
  } catch (...) { errno = ENOMEM; return -1; }
}

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

static_assert(std::numeric_limits<off_t>::is_signed,
              "portable_file_io requires a signed native file offset");
static_assert(
    std::numeric_limits<off_t>::digits >=
        std::numeric_limits<file_offset_t>::digits,
    "portable_file_io requires a native offset at least as wide as int64_t");

extern "C" FILE *file_open(const char *path, const char *mode) {
  if (path == nullptr || mode == nullptr) {
    errno = EINVAL;
    return nullptr;
  }
  return fopen(path, mode);
}

extern "C" FILE *file_open_shared(const char *path, const char *mode) {
  // fopen already permits independent opens and rename/unlink on POSIX.
  return file_open(path, mode);
}

extern "C" int file_close(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return EOF;
  }
  return fclose(file);
}

extern "C" int file_seek(FILE *file, const file_offset_t offset,
                         const int origin) {
  if (file == nullptr ||
      (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END)) {
    errno = EINVAL;
    return -1;
  }
  return fseeko(file, static_cast<off_t>(offset), origin);
}

extern "C" file_offset_t file_tell(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return static_cast<file_offset_t>(-1);
  }

  const off_t offset = ftello(file);
  if (offset == static_cast<off_t>(-1)) {
    return static_cast<file_offset_t>(-1);
  }
  if (offset > static_cast<off_t>(std::numeric_limits<file_offset_t>::max())) {
    errno = EOVERFLOW;
    return static_cast<file_offset_t>(-1);
  }
  return static_cast<file_offset_t>(offset);
}

extern "C" int file_size(FILE *file, file_offset_t *size) {
  if (file == nullptr || size == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct stat metadata{};
  if (fstat(descriptor, &metadata) != 0) {
    return -1;
  }
  if (metadata.st_size < 0 ||
      metadata.st_size >
          static_cast<off_t>(std::numeric_limits<file_offset_t>::max())) {
    errno = EOVERFLOW;
    return -1;
  }

  *size = static_cast<file_offset_t>(metadata.st_size);
  return 0;
}

extern "C" int file_last_modified(FILE *file, file_timestamp_t *timestamp) {
  if (file == nullptr || timestamp == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct stat metadata{};
  if (fstat(descriptor, &metadata) != 0) {
    return -1;
  }

  static_assert(std::numeric_limits<time_t>::is_signed,
                "portable_file_io requires a signed native timestamp");
  static_assert(
      std::numeric_limits<time_t>::digits <=
          std::numeric_limits<file_timestamp_t>::digits,
      "portable_file_io requires timestamps no wider than int64_t");

  *timestamp = static_cast<file_timestamp_t>(metadata.st_mtime);
  return 0;
}

extern "C" int file_is_regular(FILE *file) {
  if (file == nullptr) {
    errno = EINVAL;
    return -1;
  }

  const int descriptor = fileno(file);
  if (descriptor == -1) {
    return -1;
  }

  struct stat metadata{};
  if (fstat(descriptor, &metadata) != 0) {
    return -1;
  }

  return S_ISREG(metadata.st_mode) ? 1 : 0;
}

namespace {
void copy_state(const struct stat &metadata, file_state_t *state) {
  state->device = static_cast<uint64_t>(metadata.st_dev);
  state->identity = static_cast<uint64_t>(metadata.st_ino);
  state->size = static_cast<uint64_t>(metadata.st_size);
#if defined(__APPLE__)
  state->modified_seconds = metadata.st_mtime;
  state->changed_seconds = metadata.st_ctime;
#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  state->modified_subseconds = metadata.st_mtimespec.tv_nsec;
  state->changed_subseconds = metadata.st_ctimespec.tv_nsec;
#else
  state->modified_subseconds = metadata.st_mtimensec;
  state->changed_subseconds = metadata.st_ctimensec;
#endif
#else
  state->modified_seconds = metadata.st_mtim.tv_sec;
  state->modified_subseconds = metadata.st_mtim.tv_nsec;
  state->changed_seconds = metadata.st_ctim.tv_sec;
  state->changed_subseconds = metadata.st_ctim.tv_nsec;
#endif
}
}

extern "C" int file_state(FILE *file, file_state_t *state) {
  if (file == nullptr || state == nullptr) { errno = EINVAL; return -1; }
  struct stat metadata{};
  if (fstat(fileno(file), &metadata) != 0) return -1;
  copy_state(metadata, state);
  return 0;
}

extern "C" int file_path_state(const char *path, file_state_t *state) {
  if (path == nullptr || state == nullptr) { errno = EINVAL; return -1; }
  struct stat metadata{};
  if (stat(path, &metadata) != 0) return -1;
  copy_state(metadata, state);
  return 0;
}

extern "C" int file_truncate(FILE *file, const file_offset_t size) {
  if (file == nullptr || size < 0) { errno = EINVAL; return -1; }
  if (fflush(file) != 0) return -1;
  return ftruncate(fileno(file), static_cast<off_t>(size));
}

extern "C" int file_sync(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  if (fflush(file) != 0) return -1;
  return fsync(fileno(file));
}

extern "C" int file_lock_exclusive(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  struct flock lock{};
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  return fcntl(fileno(file), F_SETLK, &lock);
}

extern "C" int file_unlock(FILE *file) {
  if (file == nullptr) { errno = EINVAL; return -1; }
  struct flock lock{};
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  return fcntl(fileno(file), F_SETLK, &lock);
}

extern "C" int file_sync_directory(const char *path) {
  if (path == nullptr) { errno = EINVAL; return -1; }
  const int descriptor = open(path, O_RDONLY | O_DIRECTORY);
  if (descriptor < 0) return -1;
  const int result = fsync(descriptor);
  const int saved_errno = errno;
  const int closed = close(descriptor);
  if (result != 0) { errno = saved_errno; return result; }
  return closed;
}

#endif

extern "C" int file_read_exact_at(FILE *file, const file_offset_t offset,
                                  void *buffer, const size_t bytes) {
  if (file == nullptr || offset < 0 || (bytes != 0 && buffer == nullptr)) {
    errno = EINVAL;
    return -1;
  }
  if (bytes > static_cast<std::uint64_t>(std::numeric_limits<file_offset_t>::max() - offset)) {
    errno = EOVERFLOW;
    return -1;
  }
  if (bytes == 0) return 0;
#ifdef _WIN32
  if (file_seek(file, offset, SEEK_SET) != 0) return -1;
  if (fread(buffer, 1, bytes, file) != bytes) {
    if (!ferror(file)) errno = EIO;
    return -1;
  }
#else
  auto *destination = static_cast<char *>(buffer);
  std::size_t done = 0;
  while (done < bytes) {
    const auto count = std::min(bytes - done,
        static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()));
    const auto received = pread(fileno(file), destination + done, count,
        static_cast<off_t>(offset + static_cast<file_offset_t>(done)));
    if (received < 0 && errno == EINTR) continue;
    if (received <= 0) {
      if (received == 0) errno = EIO;
      return -1;
    }
    done += static_cast<std::size_t>(received);
  }
#endif
  return 0;
}

extern "C" int file_write_exact_at(FILE *file, const file_offset_t offset,
                                   const void *buffer, const size_t bytes) {
  if (file == nullptr || offset < 0 || (bytes != 0 && buffer == nullptr)) {
    errno = EINVAL;
    return -1;
  }
  if (bytes > static_cast<std::uint64_t>(std::numeric_limits<file_offset_t>::max() - offset)) {
    errno = EOVERFLOW;
    return -1;
  }
  if (bytes == 0) return 0;
  // POSIX fflush also discards unread buffered input on a seekable stream,
  // keeping subsequent stdio reads coherent with descriptor writes.
  if (fflush(file) != 0) return -1;
#ifdef _WIN32
  if (file_seek(file, offset, SEEK_SET) != 0) return -1;
  if (fwrite(buffer, 1, bytes, file) != bytes) return -1;
#else
  const auto *source = static_cast<const char *>(buffer);
  std::size_t done = 0;
  while (done < bytes) {
    const auto count = std::min(bytes - done,
        static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()));
    const auto written = pwrite(fileno(file), source + done, count,
        static_cast<off_t>(offset + static_cast<file_offset_t>(done)));
    if (written < 0 && errno == EINTR) continue;
    if (written <= 0) {
      if (written == 0) errno = EIO;
      return -1;
    }
    done += static_cast<std::size_t>(written);
  }
#endif
  return 0;
}
