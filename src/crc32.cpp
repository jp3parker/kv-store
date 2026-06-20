
#include "crc32.h"

#include <array>

namespace {

// Standard reflected CRC-32 polynomial
constexpr uint32_t POLYNOMIAL = 0xEDB88320u;

// Generate the lookup table once at program startup.
std::array<uint32_t, 256> generate_table() {
    std::array<uint32_t, 256> table{};

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;

        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 1)
                crc = (crc >> 1) ^ POLYNOMIAL;
            else
                crc >>= 1;
        }

        table[i] = crc;
    }

    return table;
}

// Static initialization: generated exactly once.
const std::array<uint32_t, 256> CRC_TABLE = generate_table();

} // namespace

uint32_t crc32(const void* data, std::size_t length)
{
    const auto* bytes = static_cast<const uint8_t*>(data);

    uint32_t crc = 0xFFFFFFFFu;

    for (std::size_t i = 0; i < length; ++i) {
        uint8_t index = static_cast<uint8_t>((crc ^ bytes[i]) & 0xFF);
        crc = (crc >> 8) ^ CRC_TABLE[index];
    }

    return crc ^ 0xFFFFFFFFu;
}
