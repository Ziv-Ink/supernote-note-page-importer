#pragma once
#include <cstddef>
#include <cstdint>
namespace snfiletools::internal {
std::uint32_t crc32_portable(std::uint32_t state, const char *data, std::size_t size);
std::uint32_t crc32_update(std::uint32_t state, const char *data, std::size_t size);
bool crc32_hardware_available();
}
