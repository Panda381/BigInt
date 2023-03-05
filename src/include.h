
// ****************************************************************************
//                                 
//                              Common definitions
//
// ****************************************************************************
// BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
// include.h - includes

#pragma warning(disable : 4996) // function unsafe

// ----------------------------------------------------------------------------
//                             Compilation flags
// ----------------------------------------------------------------------------
// Command line compilation flags:
//  WIN64 ... use Intel x64 optimization (64-bit mode)

#define ASM						// flag - use assembler optimization

#define BIGINT_TEMPMODE	0			// mode of temporary variables of BIGINT numbers
									//	0 = use global variables (faster, but not multithread safe)
									//	1 = use malloc allocator (slower, but multithread safe)

// flag corrections
#ifdef WIN64
#ifdef ASM
#define ASM64					// use assembler x64
#endif // ASM
#else // WIN64
#undef ASM						// ASM supported currently in x64 mode only
#endif // WIN64

#define BINFILE		"Bernoulli.bin"		// memory file
#define TMPFILE		"Bernoulli.b$$"		// temporary memory file

// ----------------------------------------------------------------------------
//                                Base data types
// ----------------------------------------------------------------------------

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;			// use "unsigned int" in 64-bit mode
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;			// use "signed int" in 64-bit mode
typedef signed long long s64;

typedef unsigned char Bool;
#define True 1
#define False 0

// NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

// ----------------------------------------------------------------------------
//                                 Constants
// ----------------------------------------------------------------------------

#define	B0 ((u8)1<<0)
#define	B1 ((u8)1<<1)
#define	B2 ((u8)1<<2)
#define	B3 ((u8)1<<3)
#define	B4 ((u8)1<<4)
#define	B5 ((u8)1<<5)
#define	B6 ((u8)1<<6)
#define	B7 ((u8)1<<7)
#define	B8 ((u16)1<<8)
#define	B9 ((u16)1<<9)
#define	B10 ((u16)1<<10)
#define	B11 ((u16)1<<11)
#define	B12 ((u16)1<<12)
#define	B13 ((u16)1<<13)
#define	B14 ((u16)1<<14)
#define	B15 ((u16)1<<15)
#define B16 ((u32)1<<16)
#define B17 ((u32)1<<17)
#define B18 ((u32)1<<18)
#define	B19 ((u32)1<<19)
#define B20 ((u32)1<<20)
#define B21 ((u32)1<<21)
#define B22 ((u32)1<<22)
#define B23 ((u32)1<<23)
#define B24 ((u32)1<<24)
#define B25 ((u32)1<<25)
#define B26 ((u32)1<<26)
#define B27 ((u32)1<<27)
#define B28 ((u32)1<<28)
#define B29 ((u32)1<<29)
#define B30 ((u32)1<<30)
#define B31 ((u32)1<<31)

#define BIT(pos) (1UL<<(pos))

///////////////////////////////////////////////////////////////////////////////
// inplace "new" operator
//		example:	new (&m_List[inx]) cText;
//					m_List[i].~cText()

inline void* operator new (size_t size, void* p)
{
	size;
	return p;
}

inline void operator delete (void* adr, void* p)
{
	adr; p;
	return;
}

#ifdef ASM64
extern "C" void MulHL8(u8* rH, u8* rL, u8 a, u8 b);
extern "C" void MulHL16(u16* rH, u16* rL, u16 a, u16 b);
extern "C" void MulHL32(u32* rH, u32* rL, u32 a, u32 b);
extern "C" void MulHL64(u64* rH, u64* rL, u64 a, u64 b);
#endif

// ----------------------------------------------------------------------------
//                                   Includes
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "main/crc.h"		// checksum
#include "main/bigint.h"	// big integers
#include "main/main.h"		// main code
