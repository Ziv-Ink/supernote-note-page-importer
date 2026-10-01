#include "internal/snfile.h"
#include <snfiletools.h>

std::int64_t get_size(const std::filesystem::path &path) {
    auto &file = open_file(path);
    return file.get_size();
}
