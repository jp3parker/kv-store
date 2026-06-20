
#include "crc32.h"

#include <gtest/gtest.h>

TEST(CRC32Test, EmptyBuffer)
{
    EXPECT_EQ(crc32(nullptr, 0), 0x00000000u);
}

TEST(CRC32Test, StandardVector123456789)
{
    const char* data = "123456789";

    EXPECT_EQ(
        crc32(data, 9),
        0xCBF43926u
    );
}

TEST(CRC32Test, Hello)
{
    std::string data = "hello";

    EXPECT_EQ(
        crc32(data.data(), data.size()),
        0x3610A686u
    );
}

TEST(CRC32Test, SameInputSameChecksum)
{
    const char* data = "The quick brown fox jumps over the lazy dog";

    uint32_t crc1 = crc32(data, std::strlen(data));
    uint32_t crc2 = crc32(data, std::strlen(data));

    EXPECT_EQ(crc1, crc2);
}

TEST(CRC32Test, OneByteDifferenceChangesChecksum)
{
    const char* data1 = "hello";
    const char* data2 = "jello";

    EXPECT_NE(
        crc32(data1, 5),
        crc32(data2, 5)
    );
}
