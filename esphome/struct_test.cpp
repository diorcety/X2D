#if defined(TEST)
#include <stdint.h>

#include <gtest/gtest.h>

#include "struct.h"

#define _STATIC_ASSERT_STRINGIZE_DETAIL(x) #x
#define _STATIC_ASSERT_STRINGIZE(x) _STATIC_ASSERT_STRINGIZE_DETAIL(x)
#define STATIC_ASSERT(X) static_assert(X, "Assertion at line " _STATIC_ASSERT_STRINGIZE(__LINE__));

/*
 * Little Endian
 */

// clang-format off
TYPEDEF_BITFIELD_BEG(TE_BITFIELD_LE, LU16)
    eBITFIELD_LE_0 = 0,
    eBITFIELD_LE_1,
    eBITFIELD_LE_2,
    eBITFIELD_LE_3,
    eBITFIELD_LE_4,
    eBITFIELD_LE_5,
    eBITFIELD_LE_6,
    eBITFIELD_LE_7,
    eBITFIELD_LE_8,
    eBITFIELD_LE_9,
    eBITFIELD_LE_10,
    eBITFIELD_LE_11,
    eBITFIELD_LE_12,
    eBITFIELD_LE_13,
    eBITFIELD_LE_14,
    eBITFIELD_LE_15,
    eBITFIELD_LE_MAX_ID
TYPEDEF_BITFIELD_END(TE_BITFIELD_LE, LU16)
    // clang-format on

    TEST(BitField, LBit)
{
    BITFIELD_VAR(TE_BITFIELD_LE, bf16_Test1);
    STATIC_ASSERT(eBITFIELD_LE_MAX_ID <= sizeof(bf16_Test1) * 8)
    uint8_t *pu8_Test1 = (uint8_t *)&bf16_Test1;

    pu8_Test1[0] = 0x0;
    pu8_Test1[1] = 0x0;
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_15), false);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_8), false);

    BITFIELD_BIT_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_7);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_7), true);
    EXPECT_EQ(pu8_Test1[0], 0x80);
    EXPECT_EQ(pu8_Test1[1], 0x00);

    pu8_Test1[0] = 0x66;
    pu8_Test1[1] = 0x00;
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_0), false);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_1), true);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_2), true);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_3), false);

    BITFIELD_BIT_CLR(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_5);
    BITFIELD_BIT_CLR(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_6);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x00);

    BITFIELD_BIT_SET_CLR(true, TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_8);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_8), true);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x01);

    BITFIELD_BIT_SET_CLR(false, TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_8);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_8), false);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x00);
}

TEST(BitField, LNumber)
{
    BITFIELD_VAR(TE_BITFIELD_LE, bf16_Test1);
    STATIC_ASSERT(eBITFIELD_LE_MAX_ID <= sizeof(bf16_Test1) * 8)
    uint8_t *pu8_Test1 = (uint8_t *)&bf16_Test1;

    pu8_Test1[0] = 0x0;
    pu8_Test1[1] = 0x0;
    EXPECT_EQ(BITFIELD_GET(TE_BITFIELD_LE, bf16_Test1), 0x0000);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_15, 0), 0x0000);

    pu8_Test1[1] = 0x10;
    EXPECT_EQ(BITFIELD_GET(TE_BITFIELD_LE, bf16_Test1), 0x1000);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_15, 0), 0x1000);

    BITFIELD_SET(TE_BITFIELD_LE, bf16_Test1, 0x01FF);
    EXPECT_EQ(pu8_Test1[0], 0xFF);
    EXPECT_EQ(pu8_Test1[1], 0x01);

    pu8_Test1[0] = 0x15;
    pu8_Test1[1] = 0xFF;
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_4, eBITFIELD_LE_2), 0x5);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_15, eBITFIELD_LE_15), 0x1);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_0, eBITFIELD_LE_0), 0x1);

    BITFIELD_SET_RANGE(TE_BITFIELD_LE, bf16_Test1, eBITFIELD_LE_15, eBITFIELD_LE_8, 0x55);
    EXPECT_EQ(pu8_Test1[0], 0x15);
    EXPECT_EQ(pu8_Test1[1], 0x55);
}

TEST(Endianess, LU64)
{
    LU64 u64_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u64_Test1;
    memset(pu8_Test1, 0, sizeof(u64_Test1));

    EXPECT_EQ(LU64_TO_UNS64(u64_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x32;
    pu8_Test1[3] = 0x5D;
    pu8_Test1[4] = 0x99;
    pu8_Test1[5] = 0x75;
    pu8_Test1[6] = 0x81;
    pu8_Test1[7] = 0xAA;
    EXPECT_EQ(LU64_TO_UNS64(u64_Test1), 12286230559997493012ull);

    u64_Test1 = UNS64_TO_LU64(10117505974174377489ull);
    EXPECT_EQ(pu8_Test1[0], 0x11);
    EXPECT_EQ(pu8_Test1[1], 0x66);
    EXPECT_EQ(pu8_Test1[2], 0x8E);
    EXPECT_EQ(pu8_Test1[3], 0x23);
    EXPECT_EQ(pu8_Test1[4], 0x14);
    EXPECT_EQ(pu8_Test1[5], 0x9A);
    EXPECT_EQ(pu8_Test1[6], 0x68);
    EXPECT_EQ(pu8_Test1[7], 0x8C);
}

TEST(Endianess, LS64)
{
    LS64 s64_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s64_Test1;
    memset(pu8_Test1, 0, sizeof(s64_Test1));

    EXPECT_EQ(LS64_TO_S64(s64_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x32;
    pu8_Test1[3] = 0x5D;
    pu8_Test1[4] = 0x99;
    pu8_Test1[5] = 0x75;
    pu8_Test1[6] = 0x81;
    pu8_Test1[7] = 0xAA;
    EXPECT_EQ(LS64_TO_S64(s64_Test1), -6160513513712058604ll);

    s64_Test1 = S64_TO_LS64(-8329238099535174127ll);
    EXPECT_EQ(pu8_Test1[0], 0x11);
    EXPECT_EQ(pu8_Test1[1], 0x66);
    EXPECT_EQ(pu8_Test1[2], 0x8E);
    EXPECT_EQ(pu8_Test1[3], 0x23);
    EXPECT_EQ(pu8_Test1[4], 0x14);
    EXPECT_EQ(pu8_Test1[5], 0x9A);
    EXPECT_EQ(pu8_Test1[6], 0x68);
    EXPECT_EQ(pu8_Test1[7], 0x8C);
}

TEST(Endianess, LU32)
{
    LU32 u32_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u32_Test1;
    memset(pu8_Test1, 0, sizeof(u32_Test1));

    EXPECT_EQ(LU32_TO_UNS32(u32_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x32;
    pu8_Test1[3] = 0x9D;
    EXPECT_EQ(LU32_TO_UNS32(u32_Test1), 2637365012u);

    u32_Test1 = UNS32_TO_LU32(4174377489u);
    EXPECT_EQ(pu8_Test1[0], 0x11);
    EXPECT_EQ(pu8_Test1[1], 0xF2);
    EXPECT_EQ(pu8_Test1[2], 0xCF);
    EXPECT_EQ(pu8_Test1[3], 0xF8);
}

TEST(Endianess, LS32)
{
    LS32 s32_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s32_Test1;
    memset(pu8_Test1, 0, sizeof(s32_Test1));

    EXPECT_EQ(LS32_TO_S32(s32_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x32;
    pu8_Test1[3] = 0x9D;
    EXPECT_EQ(LS32_TO_S32(s32_Test1), -1657602284l);

    s32_Test1 = S32_TO_LS32(-220124564l);
    EXPECT_EQ(pu8_Test1[0], 0x6C);
    EXPECT_EQ(pu8_Test1[1], 0x2A);
    EXPECT_EQ(pu8_Test1[2], 0xE1);
    EXPECT_EQ(pu8_Test1[3], 0xF2);
}

TEST(Endianess, LU24)
{
    LU24 u24_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u24_Test1;
    memset(pu8_Test1, 0, sizeof(u24_Test1));

    EXPECT_EQ(LU24_TO_UNS24(u24_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x32;
    EXPECT_EQ(LU24_TO_UNS24(u24_Test1), 3342100u);

    u24_Test1 = UNS24_TO_LU24(13627921u);
    EXPECT_EQ(pu8_Test1[0], 0x11);
    EXPECT_EQ(pu8_Test1[1], 0xF2);
    EXPECT_EQ(pu8_Test1[2], 0xCF);
}

TEST(Endianess, LS24)
{
    LS24 s24_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s24_Test1;
    memset(pu8_Test1, 0, sizeof(s24_Test1));

    EXPECT_EQ(LS24_TO_S24(s24_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    pu8_Test1[2] = 0x9D;
    EXPECT_EQ(LS24_TO_S24(s24_Test1), -6422764l);

    s24_Test1 = S24_TO_LS24(-2020756l);
    EXPECT_EQ(pu8_Test1[0], 0x6C);
    EXPECT_EQ(pu8_Test1[1], 0x2A);
    EXPECT_EQ(pu8_Test1[2], 0xE1);
}

TEST(Endianess, LU16)
{
    LU16 u16_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u16_Test1;
    memset(pu8_Test1, 0, sizeof(u16_Test1));

    EXPECT_EQ(LU16_TO_UNS16(u16_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    EXPECT_EQ(LU16_TO_UNS16(u16_Test1), 65300u);

    u16_Test1 = UNS16_TO_LU16(57489u);
    EXPECT_EQ(pu8_Test1[0], 0x91);
    EXPECT_EQ(pu8_Test1[1], 0xE0);
}

TEST(Endianess, LS16)
{
    LS16 s16_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s16_Test1;
    memset(pu8_Test1, 0, sizeof(s16_Test1));

    EXPECT_EQ(LS16_TO_S16(s16_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    pu8_Test1[1] = 0xFF;
    EXPECT_EQ(LS16_TO_S16(s16_Test1), -236l);

    s16_Test1 = S16_TO_LS16(-25864l);
    EXPECT_EQ(pu8_Test1[0], 0xF8);
    EXPECT_EQ(pu8_Test1[1], 0x9A);
}

TEST(Endianess, LU8)
{
    LU8 u8_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u8_Test1;
    memset(pu8_Test1, 0, sizeof(u8_Test1));

    EXPECT_EQ(LU8_TO_UNS8(u8_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    EXPECT_EQ(LU8_TO_UNS8(u8_Test1), 20);

    u8_Test1 = UNS8_TO_LU8(89u);
    EXPECT_EQ(pu8_Test1[0], 0x59);
}

TEST(Endianess, LS8)
{
    LS8 s8_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s8_Test1;
    memset(pu8_Test1, 0, sizeof(s8_Test1));

    EXPECT_EQ(LS8_TO_S8(s8_Test1), 0x00);

    pu8_Test1[0] = 0x84;
    EXPECT_EQ(LS8_TO_S8(s8_Test1), -124);

    s8_Test1 = S8_TO_LS8(-80);
    EXPECT_EQ(pu8_Test1[0], 0xB0);
}

/*
 * Big Endian
 */

TYPEDEF_BITFIELD_BEG(TE_BITFIELD_BE, BU16)
eBITFIELD_BE_0 = 0, eBITFIELD_BE_1, eBITFIELD_BE_2, eBITFIELD_BE_3, eBITFIELD_BE_4,

    eBITFIELD_BE_5, eBITFIELD_BE_6, eBITFIELD_BE_7, eBITFIELD_BE_8, eBITFIELD_BE_9, eBITFIELD_BE_10, eBITFIELD_BE_11,
    eBITFIELD_BE_12,

    eBITFIELD_BE_13, eBITFIELD_BE_14, eBITFIELD_BE_15,
    eBITFIELD_BE_MAX_ID TYPEDEF_BITFIELD_END(TE_BITFIELD_BE, BU16)

        TEST(BitField, BBit)
{
    BITFIELD_VAR(TE_BITFIELD_BE, bf16_Test1);
    STATIC_ASSERT(eBITFIELD_BE_MAX_ID <= sizeof(bf16_Test1) * 8)
    uint8_t *pu8_Test1 = (uint8_t *)&bf16_Test1;

    pu8_Test1[0] = 0x0;
    pu8_Test1[1] = 0x0;
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_15), false);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_8), false);

    BITFIELD_BIT_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_7);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_7), true);
    EXPECT_EQ(pu8_Test1[0], 0x00);
    EXPECT_EQ(pu8_Test1[1], 0x80);

    pu8_Test1[0] = 0x66;
    pu8_Test1[1] = 0x00;
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_8), false);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_9), true);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_10), true);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_11), false);

    BITFIELD_BIT_CLR(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_13);
    BITFIELD_BIT_CLR(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_14);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x00);

    BITFIELD_BIT_SET_CLR(true, TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_0);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_0), true);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x01);

    BITFIELD_BIT_SET_CLR(false, TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_0);
    EXPECT_EQ(BITFIELD_BIT_IS_SET(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_0), false);
    EXPECT_EQ(pu8_Test1[0], 0x06);
    EXPECT_EQ(pu8_Test1[1], 0x00);
}

TEST(BitField, BNumber)
{
    BITFIELD_VAR(TE_BITFIELD_BE, bf16_Test1);
    STATIC_ASSERT(eBITFIELD_BE_MAX_ID <= sizeof(bf16_Test1) * 8)
    uint8_t *pu8_Test1 = (uint8_t *)&bf16_Test1;

    pu8_Test1[0] = 0x0;
    pu8_Test1[1] = 0x0;
    EXPECT_EQ(BITFIELD_GET(TE_BITFIELD_BE, bf16_Test1), 0x0000);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_15, 0), 0x0000);

    pu8_Test1[1] = 0x10;
    EXPECT_EQ(BITFIELD_GET(TE_BITFIELD_BE, bf16_Test1), 0x0010);
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_15, 0), 0x0010);

    BITFIELD_SET(TE_BITFIELD_BE, bf16_Test1, 0x01FF);
    EXPECT_EQ(pu8_Test1[0], 0x01);
    EXPECT_EQ(pu8_Test1[1], 0xFF);

    pu8_Test1[0] = 0xFF;
    pu8_Test1[1] = 0x14;
    EXPECT_EQ(BITFIELD_GET_RANGE(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_4, eBITFIELD_BE_2), 0x5);

    BITFIELD_SET_RANGE(TE_BITFIELD_BE, bf16_Test1, eBITFIELD_BE_15, eBITFIELD_BE_8, 0x55);
    EXPECT_EQ(pu8_Test1[0], 0x55);
    EXPECT_EQ(pu8_Test1[1], 0x14);
}

TEST(Endianess, BU64)
{
    BU64 u64_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u64_Test1;
    memset(pu8_Test1, 0, sizeof(u64_Test1));

    EXPECT_EQ(BU64_TO_UNS64(u64_Test1), 0x00);

    pu8_Test1[0] = 0xAA;
    pu8_Test1[1] = 0x81;
    pu8_Test1[2] = 0x75;
    pu8_Test1[3] = 0x99;
    pu8_Test1[4] = 0x5D;
    pu8_Test1[5] = 0x32;
    pu8_Test1[6] = 0xFF;
    pu8_Test1[7] = 0x14;
    EXPECT_EQ(BU64_TO_UNS64(u64_Test1), 12286230559997493012ull);

    u64_Test1 = UNS64_TO_BU64(10117505974174377489ull);
    EXPECT_EQ(pu8_Test1[0], 0x8C);
    EXPECT_EQ(pu8_Test1[1], 0x68);
    EXPECT_EQ(pu8_Test1[2], 0x9A);
    EXPECT_EQ(pu8_Test1[3], 0x14);
    EXPECT_EQ(pu8_Test1[4], 0x23);
    EXPECT_EQ(pu8_Test1[5], 0x8E);
    EXPECT_EQ(pu8_Test1[6], 0x66);
    EXPECT_EQ(pu8_Test1[7], 0x11);
}

TEST(Endianess, BS64)
{
    BS64 s64_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s64_Test1;
    memset(pu8_Test1, 0, sizeof(s64_Test1));

    EXPECT_EQ(BS64_TO_S64(s64_Test1), 0x00);

    pu8_Test1[0] = 0xAA;
    pu8_Test1[1] = 0x81;
    pu8_Test1[2] = 0x75;
    pu8_Test1[3] = 0x99;
    pu8_Test1[4] = 0x5D;
    pu8_Test1[5] = 0x32;
    pu8_Test1[6] = 0xFF;
    pu8_Test1[7] = 0x14;
    EXPECT_EQ(BS64_TO_S64(s64_Test1), -6160513513712058604ll);

    s64_Test1 = S64_TO_BS64(-8329238099535174127ll);
    EXPECT_EQ(pu8_Test1[0], 0x8C);
    EXPECT_EQ(pu8_Test1[1], 0x68);
    EXPECT_EQ(pu8_Test1[2], 0x9A);
    EXPECT_EQ(pu8_Test1[3], 0x14);
    EXPECT_EQ(pu8_Test1[4], 0x23);
    EXPECT_EQ(pu8_Test1[5], 0x8E);
    EXPECT_EQ(pu8_Test1[6], 0x66);
    EXPECT_EQ(pu8_Test1[7], 0x11);
}

TEST(Endianess, BU32)
{
    BU32 u32_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u32_Test1;
    memset(pu8_Test1, 0, sizeof(u32_Test1));

    EXPECT_EQ(BU32_TO_UNS32(u32_Test1), 0x00);

    pu8_Test1[0] = 0x9D;
    pu8_Test1[1] = 0x32;
    pu8_Test1[2] = 0xFF;
    pu8_Test1[3] = 0x14;
    EXPECT_EQ(BU32_TO_UNS32(u32_Test1), 2637365012u);

    u32_Test1 = UNS32_TO_BU32(4174377489u);
    EXPECT_EQ(pu8_Test1[0], 0xF8);
    EXPECT_EQ(pu8_Test1[1], 0xCF);
    EXPECT_EQ(pu8_Test1[2], 0xF2);
    EXPECT_EQ(pu8_Test1[3], 0x11);
}

TEST(Endianess, BS32)
{
    BS32 s32_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s32_Test1;
    memset(pu8_Test1, 0, sizeof(s32_Test1));

    EXPECT_EQ(BS32_TO_S32(s32_Test1), 0x00);

    pu8_Test1[0] = 0x9D;
    pu8_Test1[1] = 0x32;
    pu8_Test1[2] = 0xFF;
    pu8_Test1[3] = 0x14;
    EXPECT_EQ(BS32_TO_S32(s32_Test1), -1657602284l);

    s32_Test1 = S32_TO_BS32(-220124564l);
    EXPECT_EQ(pu8_Test1[0], 0xF2);
    EXPECT_EQ(pu8_Test1[1], 0xE1);
    EXPECT_EQ(pu8_Test1[2], 0x2A);
    EXPECT_EQ(pu8_Test1[3], 0x6C);
}

TEST(Endianess, BU24)
{
    BU24 u24_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u24_Test1;
    memset(pu8_Test1, 0, sizeof(u24_Test1));

    EXPECT_EQ(BU24_TO_UNS24(u24_Test1), 0x00);

    pu8_Test1[0] = 0x9D;
    pu8_Test1[1] = 0x32;
    pu8_Test1[2] = 0xFF;
    EXPECT_EQ(BU24_TO_UNS24(u24_Test1), 10302207u);

    u24_Test1 = UNS24_TO_BU24(16306162u);
    EXPECT_EQ(pu8_Test1[0], 0xF8);
    EXPECT_EQ(pu8_Test1[1], 0xCF);
    EXPECT_EQ(pu8_Test1[2], 0xF2);
}

TEST(Endianess, BS24)
{
    BS24 s24_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s24_Test1;
    memset(pu8_Test1, 0, sizeof(s24_Test1));

    EXPECT_EQ(BS24_TO_S24(s24_Test1), 0x00);

    pu8_Test1[0] = 0x9D;
    pu8_Test1[1] = 0x32;
    pu8_Test1[2] = 0xFF;
    EXPECT_EQ(BS24_TO_S24(s24_Test1), -6475009l);

    s24_Test1 = S24_TO_BS24(-859862l);
    EXPECT_EQ(pu8_Test1[0], 0xF2);
    EXPECT_EQ(pu8_Test1[1], 0xE1);
    EXPECT_EQ(pu8_Test1[2], 0x2A);
}

TEST(Endianess, BU16)
{
    BU16 u16_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u16_Test1;
    memset(pu8_Test1, 0, sizeof(u16_Test1));

    EXPECT_EQ(BU16_TO_UNS16(u16_Test1), 0x00);

    pu8_Test1[0] = 0xFF;
    pu8_Test1[1] = 0x14;
    EXPECT_EQ(BU16_TO_UNS16(u16_Test1), 65300u);

    u16_Test1 = UNS16_TO_BU16(57489u);
    EXPECT_EQ(pu8_Test1[0], 0xE0);
    EXPECT_EQ(pu8_Test1[1], 0x91);
}

TEST(Endianess, BS16)
{
    BS16 s16_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s16_Test1;
    memset(pu8_Test1, 0, sizeof(s16_Test1));

    EXPECT_EQ(BS16_TO_S16(s16_Test1), 0x00);

    pu8_Test1[0] = 0xFF;
    pu8_Test1[1] = 0x14;
    EXPECT_EQ(BS16_TO_S16(s16_Test1), -236l);

    s16_Test1 = S16_TO_BS16(-25864l);
    EXPECT_EQ(pu8_Test1[0], 0x9A);
    EXPECT_EQ(pu8_Test1[1], 0xF8);
}

TEST(Endianess, BU8)
{
    BU8 u8_Test1;

    uint8_t *pu8_Test1 = (uint8_t *)&u8_Test1;
    memset(pu8_Test1, 0, sizeof(u8_Test1));

    EXPECT_EQ(BU8_TO_UNS8(u8_Test1), 0x00);

    pu8_Test1[0] = 0x14;
    EXPECT_EQ(BU8_TO_UNS8(u8_Test1), 20);

    u8_Test1 = UNS8_TO_BU8(89u);
    EXPECT_EQ(pu8_Test1[0], 0x59);
}

TEST(Endianess, BS8)
{
    BS8 s8_Test1;
    uint8_t *pu8_Test1 = (uint8_t *)&s8_Test1;
    memset(pu8_Test1, 0, sizeof(s8_Test1));

    EXPECT_EQ(BS8_TO_S8(s8_Test1), 0x00);

    pu8_Test1[0] = 0x84;
    EXPECT_EQ(BS8_TO_S8(s8_Test1), -124);

    s8_Test1 = S8_TO_BS8(-80);
    EXPECT_EQ(pu8_Test1[0], 0xB0);
}
#endif