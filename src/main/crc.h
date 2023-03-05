
// ****************************************************************************
//
//                                CRC32 Check Sum
//
// ****************************************************************************
// BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
// crc.h - CRC32 check sum header

// CRC-32 table (1 KB)
extern const u32 Crc32Tab[256];

// Calculate CRC-32
#define CRC32_INIT 0 // inverted
u32 Crc32_1(u32 crc, u8 data);
u32 Crc32_Buf(u32 crc, const void* buf, int len);
u32 Crc32(const void* buf, int len);

// Check CRC32 (returns False on error)
Bool CrcCheck();
