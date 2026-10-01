#include "crc32.h"
#include <array>
#include <cstring>
#if defined(__ANDROID__) && defined(__aarch64__)
#include <arm_acle.h>
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif
namespace snfiletools::internal {
namespace {
constexpr auto make_crc_table() {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < table.size(); ++i) {
        auto v = i;
        for (unsigned b = 0; b < 8; ++b) v = (v >> 1) ^ ((v & 1) ? 0xedb88320u : 0);
        table[i] = v;
    }
    return table;
}
#if defined(__ANDROID__) && defined(__aarch64__)
__attribute__((target("crc"))) std::uint32_t crc32_arm(std::uint32_t value, const char *data, std::size_t size) {
    while (size >= 8) {
        std::uint64_t word; std::memcpy(&word, data, 8);
        value = __crc32d(value, word); data += 8; size -= 8;
    }
    while (size--) value = __crc32b(value, static_cast<unsigned char>(*data++));
    return value;
}
#endif
}
std::uint32_t crc32_portable(std::uint32_t value, const char *data, std::size_t size) {
    static constexpr auto table = make_crc_table();
    for (std::size_t i = 0; i < size; ++i) value = table[(value ^ static_cast<unsigned char>(data[i])) & 255] ^ (value >> 8);
    return value;
}
bool crc32_hardware_available() {
#if defined(__ANDROID__) && defined(__aarch64__)
    static const bool available = (getauxval(AT_HWCAP) & HWCAP_CRC32) != 0;
    return available;
#else
    return false;
#endif
}
std::uint32_t crc32_update(std::uint32_t value, const char *data, std::size_t size) {
#if defined(__ANDROID__) && defined(__aarch64__)
    if (crc32_hardware_available()) return crc32_arm(value, data, size);
#endif
    return crc32_portable(value, data, size);
}
}
