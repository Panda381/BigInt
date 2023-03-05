
// ****************************************************************************
//                                 
//                            Big integers
//
// ****************************************************************************
// BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
// bigint.cpp - big-integers library code

#include "../include.h"

// order bit table
//                            0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
const u8 ord_bits_tab[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

// temporary numbers
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables

#define TEMP_MAX 20
bigint TempBuf[TEMP_MAX];
int TempNum = 0;

#endif

// ---------------------------------------------------------------------------
//                         Get temporary number
// ---------------------------------------------------------------------------

bigint* bigint::GetTemp()
{
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables

	if (TempNum >= TEMP_MAX) Fatal("TEMP Internal error!");
	bigint* t = &TempBuf[TempNum];
	TempNum++;
	return t;

#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc

	bigint* t = (bigint*)malloc(sizeof(bigint));
	if (s == NULL) Fatal("MEMORY Error!");
	return t;

#endif
}

// ---------------------------------------------------------------------------
//                         Free temporary number(s)
// ---------------------------------------------------------------------------

#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables

void bigint::FreeTemp(int num)
{
	if ((num < 0) || (num > TempNum)) Fatal("TEMP Internal error!");
	TempNum -= num;
}

#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc

void bigint::FreeTemp(void* temp)
{
	free(temp);
}

#endif

// ---------------------------------------------------------------------------
//                                constructor
// ---------------------------------------------------------------------------

bigint::bigint()
{
	m_Data = NULL;
	m_Num = 0;
	m_Max = 0;
	m_Sign = False;
}

// ---------------------------------------------------------------------------
//                              destructor
// ---------------------------------------------------------------------------

bigint::~bigint()
{
	free(m_Data);
}

// ---------------------------------------------------------------------------
//                  allocate array of bigint numbers
// ---------------------------------------------------------------------------

bigint* bigint::GetArr(int num)
{
	if (num <= 0) return NULL;
	bigint* arr = (bigint*)malloc(num*sizeof(bigint));
	if (arr == NULL) Fatal("GetArr: MEMORY Error!");
	bigint* d = arr;
	for (; num > 0; num--)
	{
		new (d) bigint;
		d++;
	}
	return arr;
}

// ---------------------------------------------------------------------------
//                 release array of bigint numbers
// ---------------------------------------------------------------------------

void bigint::FreeArr(bigint* arr, int num)
{
	if ((arr == NULL) || (num <= 0)) return;
	bigint* d = arr;
	for (; num > 0; num--)
	{
		d->~bigint();
		d++;
	}
	free(arr);
}

// ---------------------------------------------------------------------------
//      resize array of bigint numbers (returns new address of array)
// ---------------------------------------------------------------------------

bigint* bigint::ResizeArr(bigint* arr, int oldnum, int newnum)
{
	// create new array
	if (arr == NULL) return bigint::GetArr(newnum);

	// delete array
	if (newnum <= 0)
	{
		bigint::FreeArr(arr, oldnum);
		return NULL;
	}

	// array is equal
	if (oldnum == newnum) return arr;

	// upscale array
	if (newnum > oldnum)
	{
		arr = (bigint*)realloc(arr, (size_t)newnum*sizeof(bigint));
		if (arr == NULL) Fatal("ResizeArr: MEMORY Error!");
		bigint* d = &arr[oldnum];
		int i;
		for (i = oldnum; i < newnum; i++)
		{
			new (d) bigint;
			d++;
		}
		return arr;
	}

	// downscale array
	bigint* d = &arr[newnum];
	int i;
	for (i = newnum; i < oldnum; i++)
	{
		d->~bigint();
		d++;
	}
	arr = (bigint*)realloc(arr, newnum*sizeof(bigint));
	if (arr == NULL) Fatal("ResizeArr: MEMORY Error!");
	return arr;
}

// ---------------------------------------------------------------------------
//        set number of segments, segments can stay uninitialized
// ---------------------------------------------------------------------------

void bigint::SetSize(int num)
{
	// change number of segments
	if (num != m_Num)
	{
		// increase number of segments
		if (num > m_Num)
		{
			// increase buffer size
			if (num > m_Max)
			{
				free(m_Data);	// delete old buffer
				int max = num + 8; // with sime reserve
				BIGINT_BASE* data = (BIGINT_BASE*)malloc(max*sizeof(BIGINT_BASE)); // create new buffer
				if (data == NULL) Fatal("SetSize: MEMORY Error!");
				m_Data = data;
				m_Max = max;
			}
		}

		// set new size
		m_Num = num;
	}
	if (num == 0) m_Sign = False;
}

// ---------------------------------------------------------------------------
//                   resize number of segments
// ---------------------------------------------------------------------------

void bigint::Resize(int num)
{
	// change number of segments
	if (num != m_Num)
	{
		// increase number of segments
		if (num > m_Num)
		{
			// increase buffer size
			if (num > m_Max)
			{
				int max = num + 8; // with sime reserve
				BIGINT_BASE* data = (BIGINT_BASE*)realloc(m_Data, max*sizeof(BIGINT_BASE));
				if (data == NULL) Fatal("Resize: MEMORY Error!");
				m_Data = data;
				m_Max = max;
			}

			// clear new segments
			int n = num - m_Num;
			BIGINT_BASE* d = &m_Data[m_Num];
			for (; n > 0; n--) *d++ = 0;
		}

		// set new size
		m_Num = num;
	}
	if (num == 0) m_Sign = False;
}

// ---------------------------------------------------------------------------
//                  reduce - delete zero segments
// ---------------------------------------------------------------------------

void bigint::Reduce()
{
	int n = m_Num;
	BIGINT_BASE* d = &m_Data[n];
	for (; n > 0; n--)
	{
		d--;
		if (*d != 0) break;
	}
	m_Num = n;
	if (n == 0) m_Sign = False;
}

// ---------------------------------------------------------------------------
//                  long multiply rH:rL <- a*b
// ---------------------------------------------------------------------------

#if !defined(ASM64)
void bigint::MulHL(BIGINT_BASE* rH, BIGINT_BASE* rL, BIGINT_BASE a, BIGINT_BASE b)
{
#if BIGINT_BASE_BITS == 8 // u8

	u16 k = (u16)a*b;
	*rL = (u8)k;
	*rH = (u8)(k >> 8);

#elif BIGINT_BASE_BITS == 16 // u16

	u32 k = (u32)a*b;
	*rL = (u16)k;
	*rH = (u16)(k >> 16);

#elif BIGINT_BASE_BITS == 32 // u32

	u64 k = (u64)a*b;
	*rL = (u32)k;
	*rH = (u32)(k >> 32);

#else // u64

	//       aHaL
	//     x bHbL
	// ----------
	//       aLbL ...   k0
	//     aLbH   ...  k1
	//     aHbL   ...  k2
	//   aHbH     ... k3

	// prepare elements
	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	// multiply elements (max. FFFF * FFFF = FFFE0001)
	u64 k0 = (u64)aL*bL;
	u64 k1 = (u64)aL*bH;
	u64 k2 = (u64)aH*bL;
	u64 k3 = (u64)aH*bH;

	// add komponents
	k1 += (k0 >> 32); // max. FFFE0001 + FFFE = FFFEFFFF, no carry yet
	k1 += k2; // max. FFFEFFFF + FFFE0001 = 1FFFD0000, it can carry
	if (k1 < k2) k3 += (u64)1 << 32; // add carry, FFFE0001 + 10000 = FFFF0001, no carry

	// result, max. FFFFFFFF * FFFFFFFF = FFFFFFFE:00000001
	*rL = (k1 << 32) + (u32)k0; // result low, FFFF0000 + FFFF = FFFFFFFF, no carry
	*rH = k3 + (k1 >> 32); // result high, FFFF0001 + FFFD = FFFFFFFE, no carry 

#endif

}
#endif // ASM64

// ---------------------------------------------------------------------------
//                  copy from another number
// ---------------------------------------------------------------------------

void bigint::Copy(const bigint* num)
{
	// identical number
	if (this == num) return;

	// set size of destination buffer
	this->SetSize(num->m_Num);

	// copy data
	memcpy(this->m_Data, num->m_Data, num->m_Num*sizeof(BIGINT_BASE));

	// copy sign
	this->m_Sign = num->m_Sign;
}

void bigint::Copy(const cbigint* num)
{
	// get number size in u64
	int n = num->m_Num;

#if BIGINT_BASE_BITS == 32
	n *= 2;
#elif BIGINT_BASE_BITS == 16
	n *= 4;
#elif BIGINT_BASE_BITS == 8
	n *= 8;
#endif

	// set size of destination buffer
	this->SetSize(n);

	// copy data
	memcpy(this->m_Data, num->m_Data, n*sizeof(BIGINT_BASE));

	// copy sign
	this->m_Sign = num->m_Sign;

	this->Reduce();
}

// ---------------------------------------------------------------------------
//                      exchange numbers
// ---------------------------------------------------------------------------

void bigint::Exch(bigint* num)
{
	BIGINT_BASE* d = this->m_Data;
	int n = this->m_Num;
	int max = this->m_Max;
	Bool sign = this->m_Sign;

	this->m_Data = num->m_Data;
	this->m_Num = num->m_Num;
	this->m_Max = num->m_Max;
	this->m_Sign = num->m_Sign;

	num->m_Data = d;
	num->m_Num = n;
	num->m_Max = max;
	num->m_Sign = sign;
}

// ---------------------------------------------------------------------------
// compare numbers (result: 1 if num1 > num2, 0 if num1 == num2, -1 if num1 < num2)
// ---------------------------------------------------------------------------

int bigint::Comp(const bigint* num1, const bigint* num2)
{
	// pointers are equal
	if (num1 == num2) return 0;

	// get signs
	Bool sign1 = num1->m_Sign;
	Bool sign2 = num2->m_Sign;

	// compare signs
	if (!sign1 && sign2) return 1; // num1 (>= 0) > num2 (< 0)
	if (sign1 && !sign2) return -1; // num1 (< 0) < num2 (>= 0)

	// signs are equal
	if (sign1)
		return - bigint::CompAbs(num1, num2);
	else
		return bigint::CompAbs(num1, num2);
}

// ---------------------------------------------------------------------------
// compare absolute value of numbers (result: 1 if num1 > num2, 0 if num1 == num2, -1 if num1 < num2)
// ---------------------------------------------------------------------------

int bigint::CompAbs(const bigint* num1, const bigint* num2)
{
	// pointers are equal
	if (num1 == num2) return 0;

	// get source number of segments
	int n1 = num1->m_Num;
	int n2 = num2->m_Num;

	// compare lengths
	if (n1 > n2) return 1;		// num1 > num2
	if (n1 < n2) return -1;		// num1 < num2

	// lengths are equal, compare numbers
	BIGINT_BASE a1, a2, *s1, *s2;
	s1 = &num1->m_Data[n1];
	s2 = &num2->m_Data[n1];
	for (; n1 > 0; n1--)
	{
		s1--;
		s2--;
		a1 = *s1;
		a2 = *s2;
		if (a1 > a2) return 1;		// num1 > num2
		if (a1 < a2) return -1;		// num1 < num2
	}

	// numbers are equal
	return 0;
}

// ---------------------------------------------------------------------------
//     get bit length of the number (= 1 + bit position of highest '1')
// ---------------------------------------------------------------------------

int bigint::BitLen() const
{
	// get number of whole segments
	int n = this->m_Num;
	if (n == 0) return 0;
	n--;
	int pos = n * BIGINT_BASE_BITS;

	// prepare highest segment
	BIGINT_BASE a = this->m_Data[n];

#if BIGINT_BASE_BITS == 64
	if (a >= 0x100000000ULL)
	{
		pos += 32;
		a >>= 32;
	}
#endif

#if BIGINT_BASE_BITS >= 32
	if (a >= 0x10000UL)
	{
		pos += 16;
		a >>= 16;
	}
#endif	

#if BIGINT_BASE_BITS >= 16
	if (a >= 0x100)
	{
		pos += 8;
		a >>= 8;
	}
#endif	

	if (a >= 0x10)
	{
		pos += 4;
		a >>= 4;
	}

	return ord_bits_tab[a] + pos;
}

// ---------------------------------------------------------------------------
//          get number of lower bits '0' (trailing zeros)
// ---------------------------------------------------------------------------

int bigint::Bit0() const
{
	// zero number
	int num = m_Num;
	if (num == 0) return 0;

	// count whole segments
	BIGINT_BASE a, *s;
	int n = 0;
	s = m_Data;
	for (; num > 0; num--)
	{
		a = *s++;
		if (a != 0) break;
		n += BIGINT_BASE_BITS;
	}

	// count bits in last non-zero segment
#if BIGINT_BASE_BITS == 64
	if ((a & 0x00000000FFFFFFFFULL) == 0)
	{
		n += 32;
		a >>= 32;
	}
#endif

#if BIGINT_BASE_BITS >= 32
	if ((a & 0x0000FFFFUL) == 0)
	{
		n += 16;
		a >>= 16;
	}
#endif

#if BIGINT_BASE_BITS >= 16
	if ((a & 0x00FF) == 0)
	{
		n += 8;
		a >>= 8;
	}
#endif

	if ((a & 0x0F) == 0)
	{
		n += 4;
		a >>= 4;
	}

	if ((a & 0x03) == 0)
	{
		n += 2;
		a >>= 2;
	}

	if ((a & 0x01) == 0) n += 1;

	return n;
}

// ---------------------------------------------------------------------------
//           shift number 1 bit left (= multiply * 2)
// ---------------------------------------------------------------------------

void bigint::ShiftL1()
{
	int i = m_Num;					// number of segments
	BIGINT_BASE* s = &m_Data[0];	// pointer to first segment
	u8 carry;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

	BIGINT_BASE b;
	u8 carry2;
	carry = 0;

	for (; i > 0; i--)
	{
		b = *s;						// load mantissa segment
		carry2 = carry;				// save input carry
		carry = ((b & BIGINT_BASE_LAST) == 0) ? 0 : 1; // get new carry
		b = (b << 1) | carry2;		// shift and add input carry		
		*s++ = b;					// set new segment
	}

#else

	carry = ShiftL1Str_x64(i, s, 0);

#endif

	// carry
	if (carry)
	{
		i = m_Num;
		this->Resize(i+1);
		this->m_Data[i] = 1;
	}
}

// ---------------------------------------------------------------------------
//             shift number 1 bit right (= divide / 2)
// ---------------------------------------------------------------------------

void bigint::ShiftR1()
{
	int i = m_Num;					// number of segments
	BIGINT_BASE* s = &m_Data[i];	// pointer after end of data

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

	BIGINT_BASE b;
	u8 carry2;
	u8 carry = 0;

	for (; i > 0; i--)
	{
		s--;						// shift pointer
		b = *s;						// load segment
		carry2 = carry;				// save input carry
		carry = (u8)(b & 1);		// get new carry
		b >>= 1;					// shift
		if (carry2) b |= BIGINT_BASE_LAST; // add input carry
		*s = b;					// set new segment
	}

#else

	ShiftR1Str_x64(i, s, 0);

#endif

	this->Reduce();
}

// ---------------------------------------------------------------------------
//                    shift number more bits left
// ---------------------------------------------------------------------------

void bigint::ShiftL(int shift)
{
	// invalid number of shifts
	if (shift <= 0) return;

	// split number of shifts
	int shiftn = shift / BIGINT_BASE_BITS;	// number of shifts by whole segments
	int shiftb = shift & (BIGINT_BASE_BITS-1); // number of shifts by remaining bits

	// resize number
	int n = m_Num;
	int n2 = n + shiftn + 1;
	this->Resize(n2);

	// prepare pointers
	BIGINT_BASE* s = &m_Data[n]; // source after last segment
	BIGINT_BASE* d = &m_Data[n2]; // destination after last segment

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

	// shift by whole segments
	int i;
	if (shiftb == 0)
	{
		*--d = 0;
		for (i = n; i > 0; i--) *--d = *--s;
		for (i = shiftn; i > 0; i--) *--d = 0;
	}

	// shift by bits
	else
	{
		// store first segment
		BIGINT_BASE segH = *--s;
		int shiftb2 = BIGINT_BASE_BITS - shiftb;
		*--d = (segH >> shiftb2);

		// shift
		BIGINT_BASE segL;
		for (i = n - 1; i > 0; i--)
		{
			segL = *--s;
			*--d = (segH << shiftb) | (segL >> shiftb2);
			segH = segL;
		}

		// store last segment
		*--d = (segH << shiftb);

		// clear remaining segments
		for (i = shiftn; i > 0; i--) *--d = 0;
	}

#else

	ShiftLStr_x64(n, shift, d, s);

#endif

	this->Reduce();
}

// ---------------------------------------------------------------------------
//                   shift number more bits right
// ---------------------------------------------------------------------------

void bigint::ShiftR(int shift)
{
	// limit number of shifts
	int n = m_Num;
	int mx = n*BIGINT_BASE_BITS;
	if (shift > mx) shift = mx;
	if (shift <= 0) return;

	// split number of shifts
	int shiftn = shift / BIGINT_BASE_BITS;	// number of shifts by whole segments
	int shiftb = shift & (BIGINT_BASE_BITS - 1); // number of shifts by remaining bits

	// prepare pointers
	BIGINT_BASE* d = &m_Data[0]; // destination first segment
	BIGINT_BASE* s = &m_Data[shiftn]; // next segment

	// shift by whole segments
	int i;
	n -= shiftn;
	if (shiftb == 0)
	{
		for (i = n; i > 0; i--) *d++ = *s++;
		m_Num = n;
		return;
	}

	// shift
	BIGINT_BASE segH;
	BIGINT_BASE segL = *s;
	int shiftb2 = BIGINT_BASE_BITS - shiftb;
	for (i = n - 1; i > 0; i--)
	{
		s++;
		segH = *s;
		*d++ = (segH << shiftb2) | (segL >> shiftb);
		segL = segH;
	}

	// store last segment
	segL >>= shiftb;
	*d = segL;

	m_Num = n;
	this->Reduce();
}

// ---------------------------------------------------------------------------
//                check if number is equal integer value
// ---------------------------------------------------------------------------

Bool bigint::EquInt(BIGINT_BASES num)
{
	if (num == 0)
		return this->IsZero();
	else if (m_Num != 1)
		return False;
	else
		return num == m_Data[0];
}

// ---------------------------------------------------------------------------
//                             set value 0
// ---------------------------------------------------------------------------

void bigint::Set0()
{
	m_Num = 0;
	m_Sign = False;
}

// ---------------------------------------------------------------------------
//                             set value 1
// ---------------------------------------------------------------------------

void bigint::Set1()
{
	this->SetSize(1);
	m_Data[0] = 1;
	m_Sign = False;
}

// ---------------------------------------------------------------------------
//                            set integer value
// ---------------------------------------------------------------------------

void bigint::SetInt(BIGINT_BASES num)
{
	if (num == 0)
		this->Set0();
	else
	{
		this->SetSize(1);
		Bool sign = (num < 0);
		m_Sign = sign;
		if (sign) num = -num;
		m_Data[0] = (BIGINT_BASE)num;
	}
}

// ---------------------------------------------------------------------------
// add/sub two numbers (this = num1 +- num2, operands and destination can be the same)
// ---------------------------------------------------------------------------

void bigint::AddSub(const bigint* num1, const bigint* num2, Bool sub)
{
	// get source number of segments
	int n1 = num1->m_Num;
	int n2 = num2->m_Num;

	// num2 is zero, copy num1 to destination
	if (n2 == 0)
	{
		if (this != num1)
		{
			this->SetSize(n1);
			if (n1 > 0) memcpy(this->m_Data, num1->m_Data, n1*sizeof(BIGINT_BASE));
			this->m_Sign = num1->m_Sign;
		}
		return;
	}

	// num1 is zero, copy num2 to destination
	if (n1 == 0)
	{
		if (this != num2)
		{
			this->SetSize(n2);
			if (n2 > 0) memcpy(this->m_Data, num2->m_Data, n2*sizeof(BIGINT_BASE));
			this->m_Sign = num2->m_Sign;
		}
		if (sub) this->Neg();
		return;
	}

	// resize destination (do not use SetSize - destination can be one of operands)
	int n = n1;
	if (n < n2) n = n2;
	this->Resize(n);

	// get signs
	Bool sign1 = num1->m_Sign;
	Bool sign2 = num2->m_Sign;
	if (sub) sign2 = !sign2;

	// prepare data pointers
	const BIGINT_BASE* s1 = num1->m_Data;
	const BIGINT_BASE* s2 = num2->m_Data;	
	BIGINT_BASE* d = this->m_Data;

	// signs are equal, do addition (can be positive or negative)
	if (sign1 == sign2)
	{
		// set sign
		this->m_Sign = sign1;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

		BIGINT_BASE a, b;

		// prepare length of common part
		int n0 = n1;
		if (n0 > n2) n0 = n2;

		// add common part
		u8 carry = 0;
		for (; n0 > 0; n0--)
		{
			a = *s1 + carry;
			carry = (a < carry) ? 1 : 0;
			b = *s2;
			a += b;
			carry += (a < b) ? 1 : 0;
			*d = a;
			d++;
			s1++;
			s2++;
		}

		// add rest of num1
		n0 = n1 - n2;
		for (; n0 > 0; n0--)
		{
			a = *s1 + carry;
			carry = (a < carry) ? 1 : 0;
			*d = a;
			d++;
			s1++;
		}

		// add rest of num2
		n0 = n2 - n1;
		for (; n0 > 0; n0--)
		{
			a = *s2 + carry;
			carry = (a < carry) ? 1 : 0;
			*d = a;
			d++;
			s2++;
		}

#else

		u8 carry = (u8)AddStr_x64(n1, n2, d, s1, s2);

#endif

		// carry, increase size of the number
		if (carry)
		{
			this->Resize(n+1);
			this->m_Data[n] = 1;
		}
	}
	else
	{
		// if num1 is negative and num2 positive, exchange them
		if (sign1)
		{
			n1 = num2->m_Num;
			n2 = num1->m_Num;
			s1 = num2->m_Data;
			s2 = num1->m_Data;	
		}

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

		BIGINT_BASE a, b;

		// prepare length of common part
		int n0 = n1;
		if (n0 > n2) n0 = n2;

		// subtract num2 from num1, common part (dst = num1 - num2)
		u8 carry = 0;
		for (; n0 > 0; n0--)
		{
			b = *s2 + carry;
			carry = (b < carry) ? 1 : 0;
			a = *s1;
			carry += (a < b) ? 1 : 0;
			*d = a - b;
			d++;
			s1++;
			s2++;
		}

		// subtract 0 from rest of num1 (dst = num1 - 0)
		n0 = n1 - n2;
		for (; n0 > 0; n0--)
		{
			b = carry;
			a = *s1;
			carry = (a < b) ? 1 : 0;
			*d = a - b;
			d++;
			s1++;
		}

		// subtract rest of num2 from 0 (dst = 0 - num2)
		n0 = n2 - n1;
		for (; n0 > 0; n0--)
		{
			b = *s2 + carry;
			carry = (b < carry) ? 1 : 0;
			carry += (0 < b) ? 1 : 0;
			*d = 0 - b;
			d++;
			s2++;
		}

#else

		u8 carry = (u8)SubStr_x64(n1, n2, d, s1, s2);

#endif

		// check result sign
		m_Sign = False;
		if (carry)
		{
			m_Sign = True;
			d = this->m_Data;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

			BIGINT_BASE a;

			// negate number - 1st step, invert segments
			int n0;
			for (n0 = n; n0 > 0; n0--)
			{
				*d = ~*d;
				d++;
			}

			// negate number - 2nd step, increment
			d = this->m_Data;
			for (n0 = n; n0 > 0; n0--)
			{
				a = *d + 1;
				*d = a;
				if (a != 0) break;
				d++;
			}

#else

		NegStr_x64(n, d);

#endif

		}
	}

	// reduce destination
	this->Reduce();
}

// ---------------------------------------------------------------------------
// multiply two numbers (this = num1 * num2, operands and destintion can be the same)
// ---------------------------------------------------------------------------

void bigint::Mul(const bigint* num1, const bigint* num2)
{
	// get source number of segments
	int n1 = num1->m_Num;
	int n2 = num2->m_Num;

	// some operand is zero, result will be zero
	if ((n1 == 0) || (n2 == 0))
	{
		this->Set0();
		return;
	}

	// get temporary buffer	
	bigint* temp = this->GetTemp();

	// resize temporary result number
	int n = n1 + n2;
	temp->SetSize(n);

	// multiply loops
	BIGINT_BASE a;
	const BIGINT_BASE *s;
	BIGINT_BASE *d;
	int i;

	// 1st loop - multiply and store
	a = num1->m_Data[0];
	d = temp->m_Data;
	s = num2->m_Data;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

	BIGINT_BASE rH, rL, carry;
	int j;

	carry = 0;
	for (j = n2; j > 0; j--)
	{
		bigint::MulHL(&rH, &rL, a, *s);
		rL += carry;
		if (rL < carry) rH++;
		*d = rL;
		carry = rH;
		d++;
		s++;
	}
	*d = carry;

#else

	MulSetStr_x64(n2, a, d, s); // stores carry to dst[num]
	
#endif

	// next loops - multiply and add
	for (i = 1; i < n1; i++)
	{
		a = num1->m_Data[i];
		d = &temp->m_Data[i];
		s = num2->m_Data;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

		BIGINT_BASE rH, rL, b, carry;
		int j;

		carry = 0;
		for (j = n2; j > 0; j--)
		{
			bigint::MulHL(&rH, &rL, a, *s);
			rL += carry;
			if (rL < carry) rH++;
			b = *d;
			rL += b;
			*d = rL;
			if (rL < b) rH++;
			carry = rH;
			d++;
			s++;
		}
		*d = carry;

#else

		MulAddStr_x64(n2, a, d, s); // stores carry to dst[num]
	
#endif

	}

	// set result data
	Bool sign = num1->m_Sign ^ num2->m_Sign;
	this->Exch(temp);
	this->m_Sign = sign;

	// reduce destination
	this->Reduce();

#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(1);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp);
#endif
}

// ---------------------------------------------------------------------------
// divide two numbers (this = num1 / num2, rem = remainder or NULL if not required)
// ---------------------------------------------------------------------------

void bigint::Div(const bigint* num1, const bigint* num2, bigint* rem /* = NULL */)
{
	// get source number of segments
	int n1 = num1->m_Num;
	int n2 = num2->m_Num;

	// prepare result sign (sign2: result, sign1: remainder)
	Bool sign1 = num1->m_Sign;
	Bool sign2 = num2->m_Sign;
	sign2 ^= sign1;

	// some operand is 0, or abs(num1) < abs(num2) ... result will be 0 + num1
	int res = num1->CompAbs(num2);
	if ((n1 == 0) || (n2 == 0) || (res < 0))
	{
		if (rem != NULL) rem->Copy(num1);
		this->Set0();
		return;
	}

	// if abs(num1) == abs(num2), result will be (-)1 + 0
	if (res == 0)
	{
		if (rem != NULL) rem->Set0();
		this->Set1();
		this->m_Sign = sign2;
		return;
	}

// here is abs(num1) > abd(num2)

	// num2 is 1 segment
	if (n2 == 1)
	{
		BIGINT_BASE b = num2->m_Data[0];

		// both numbers are small
		if (n1 == 1)
		{
			BIGINT_BASE a = num1->m_Data[0];

			BIGINT_BASE c = a / b;
			this->SetSize(1);
			this->m_Data[0] = c;
			this->m_Sign = sign2;
			this->Reduce();

			if (rem != NULL)
			{
				rem->SetSize(1);
				rem->m_Data[0] = a - c*b;
				rem->m_Sign = sign1;
				rem->Reduce();
			}
			return;
		}

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

#if BIGINT_BASE_BITS >= 32
		// fast division by small number
		if ((u64)b < 0x0000000100000000ULL)
		{
			this->Resize(n1);
			this->m_Sign = sign2;
			const u32* s = (u32*)&num1->m_Data[n1];
			u32* d = (u32*)&this->m_Data[n1];
			u64 a, carry;
			carry = 0;
#if BIGINT_BASE_BITS == 64
			n1 *= 2;
#endif
			for (; n1 > 0; n1--)
			{
				s--;
				d--;
				carry = *s | (carry << 32);
				a = carry / b;
				*d = (u32)a;
				carry -= a*b;
			}

			if (rem != NULL)
			{
				rem->SetSize(1);
				rem->m_Data[0] = (BIGINT_BASE)carry;
				rem->m_Sign = sign1;
				rem->Reduce();
			}
			return;
		}
#endif

#else

		this->Resize(n1);
		this->m_Sign = sign2;
		const u64* s = &num1->m_Data[n1];
		u64* d = &this->m_Data[n1];

		u64 carry = DivStr_x64(n1, b, d, s);

		this->Reduce();

		if (rem != NULL)
		{
			rem->SetSize(1);
			rem->m_Data[0] = carry;
			rem->m_Sign = sign1;
			rem->Reduce();
		}
		return;

#endif

	}

	// get temporary buffers
	bigint* temp1 = this->GetTemp();
	bigint* temp2 = this->GetTemp();

	// copy operands
	temp1->Copy(num1);
	temp1->Abs();
	temp2->Copy(num2);
	temp2->Abs();

	// get bit length of numbers (here is len1 >= len2)
	int len1 = temp1->BitLen();
	int len2 = temp2->BitLen();

	// shift left num2 to the same bit position as num1
	len1 -= len2;
	len2 += len1;
	temp2->ShiftL(len1);
	temp1->Resize(temp1->m_Num+1);
	temp2->Resize(temp1->m_Num);

	// prepare accumulator
	int n = ((len1+1) + BIGINT_BASE_BITS-1)/BIGINT_BASE_BITS;
	this->SetSize(n+1);
	memset(this->m_Data, 0, (n+1)*sizeof(BIGINT_BASE));

	// prepare destination address and mask
	BIGINT_BASE* d = &this->m_Data[n-1];
	BIGINT_BASE m = (BIGINT_BASE)1 << (len1 % BIGINT_BASE_BITS);

	// divide numbers (len2 = current bit length)
	for (; len1 >= 0; len1--)
	{
		// set new length of numbers
		temp1->m_Num = (len2+1+BIGINT_BASE_BITS-1) / BIGINT_BASE_BITS;
		temp2->m_Num = temp1->m_Num;

		// if num1 >= num2
		if (temp1->CompAbs(temp2) >= 0)
		{
			// set result bit
			*d |= m;

			// subtract num2 from num1
			BIGINT_BASE* d2 = temp1->m_Data;
			const BIGINT_BASE* s = temp2->m_Data;
			int n = temp2->m_Num;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

			BIGINT_BASE a, b;
			u8 carry = 0;
			for (; n > 0; n--)
			{
				b = *s + carry;
				carry = (b < carry) ? 1 : 0;
				a = *d2;
				carry += (a < b) ? 1 : 0;
				*d2 = a - b;
				d2++;
				s++;
			}

#else

			DivSubStr_x64(n, d2, s);

#endif

		}

		// shift mask
		m >>= 1;
		if (m == 0)
		{
			d--;
			m = BIGINT_BASE_LAST;
		}

		// shift num2 1 bit right
		temp2->ShiftR1();
		len2--;
	}

	// reduce result
	this->m_Sign = sign2;
	this->Reduce();

	// store remainder
	if (rem != NULL)
	{
		rem->Copy(temp1);
		rem->m_Sign = sign1;
		rem->Reduce();
	}

	// release temporary buffers
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(2);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp1);
	FreeTemp(temp2);
#endif
}

// ---------------------------------------------------------------------------
//      get modulo - remainder (this = this % num, remainder will always be >= 0)
// ---------------------------------------------------------------------------

void bigint::Mod(const bigint* num)
{
	// absolute value of 'this'
	this->Abs();

	// if abs(num1) < abs(num2), result will be num1
	int res = this->CompAbs(num);
	if (res == -1) return;

	// if abs(num1) == abs(num2), result will be 0
	if ((res == 0) || this->IsZero() || num->IsZero())
	{
		this->Set0();
		return;
	}

// here is abs(num1) > abd(num2)

	// num is 1 segment
	if (num->m_Num == 1)
	{
		BIGINT_BASE a = num->m_Data[0];

		// special case: num is power of 2
		if ((a & (a-1)) == 0)
		{
			this->m_Data[0] &= (a - 1);
			this->m_Num = 1;
			this->Reduce();
			return;
		}

		// both numbers are small
		if (this->m_Num == 1)
		{
			this->m_Data[0] = this->m_Data[0] % a;
			this->Reduce();
			return;
		}

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

#if BIGINT_BASE_BITS >= 32
		// fast division by small number
		if ((u64)a < 0x0000000100000000ULL)
		{
			int n = this->m_Num;
			const u32* s = (const u32*)&this->m_Data[n];
			u64 b, carry;
			carry = 0;
#if BIGINT_BASE_BITS == 64
			n *= 2;
#endif
			for (; n > 0; n--)
			{
				s--;
				carry = *s | (carry << 32);
				b = carry / a;
				carry -= b*a;
			}

			this->m_Data[0] = (BIGINT_BASE)carry;
			this->m_Num = 1;
			this->Reduce();
			return;
		}
#endif

#else

		int n = this->m_Num;
		const u64* s = (const u64*)&this->m_Data[n];

		u64 carry = ModStr_x64(n, a, s);

		this->m_Data[0] = (BIGINT_BASE)carry;
		this->m_Num = 1;
		this->Reduce();
		return;

#endif

	}

	// get temporary buffer
	bigint* temp = this->GetTemp();

	// temporary copy of num
	temp->Copy(num);
	temp->Abs();

	// get bit length of numbers (here is len1 >= len2)
	int len1 = this->BitLen();
	int len2 = temp->BitLen();

	// shift left num2 to the same bit position
	len1 -= len2;
	len2 += len1;
	temp->ShiftL(len1);
	temp->Resize(temp->m_Num+1);
	this->Resize(temp->m_Num);

	// divide numbers
	for (; len1 >= 0; len1--)
	{
		// set new length of numbers
		temp->m_Num = (len2+1+BIGINT_BASE_BITS-1) / BIGINT_BASE_BITS;
		this->m_Num = temp->m_Num;

		// if num1 >= num2
		if (this->CompAbs(temp) >= 0)
		{
			// subtract num2 from num1
			const BIGINT_BASE* s = temp->m_Data;
			BIGINT_BASE* d = this->m_Data;
			int n = temp->m_Num;

#if !defined(ASM64) || (BIGINT_BASE_BITS != 64)

			BIGINT_BASE a, b;
			u8 carry = 0;
			for (; n > 0; n--)
			{
				b = *s + carry;
				carry = (b < carry) ? 1 : 0;
				a = *d;
				carry += (a < b) ? 1 : 0;
				*d = a - b;
				d++;
				s++;
			}

#else

			DivSubStr_x64(n, d, s);

#endif

		}

		// shift num2 1 bit right
		temp->ShiftR1();
		len2--;
	}

	// reduce destination
	this->Reduce();

	// release temporary buffer
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(1);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp);
#endif
}

// ---------------------------------------------------------------------------
//        find greatest common divisor GCD (result will always be >= 0)
// ---------------------------------------------------------------------------
// Binary GCD algorithm (Stein's algorithm) https://en.wikipedia.org/wiki/Binary_GCD_algorithm
// Binary Euclidean algorithm https://en.wikipedia.org/wiki/Euclidean_algorithm
// https://lemire.me/blog/2013/12/26/fastest-way-to-compute-the-greatest-common-divisor/

#if 0 // 1 = use binary GCD, 0 = use Euclidean (speeds are comparable)

// binary GCD
void bigint::GCD(const bigint* num1, const bigint* num2)
{
	// some number is 0, result will be 1
	if (num1->IsZero() || num2->IsZero())
	{
		this->Set1();
		return;
	}

	// both numbers are small, use fast integer
	if ((num1->m_Num == 1) && (num2->m_Num == 1))
	{
		BIGINT_BASE a = num1->m_Data[0];
		BIGINT_BASE b = num2->m_Data[0];
		for (;;)
		{
			a = a % b;
			if (a == 0)
			{
				a = b;
				break;
			}

			b = b % a;
			if (b == 0) break;
		}

		this->SetSize(1);
		this->m_Data[0] = a;
		this->m_Sign = False;
		return;
	}

	// get temporary buffer
	bigint* temp = this->GetTemp();

	// copy of numbers
	temp->Copy(num2);
	this->Copy(num1);

	// absolute value
	temp->Abs();
	this->Abs();

	// get common divisor of 2
	int shift = temp->Bit0();
	int shift2 = this->Bit0();

	// delete multiplies of 2
	temp->ShiftR(shift);
	this->ShiftR(shift2);
	if (shift > shift2) shift = shift2;

	// search loop
	for (;;)
	{
		if (this->Comp(temp) > 0) // this > temp
		{
			this->Sub(temp);
			if (this->IsZero())
			{
				this->Copy(temp);
				break;
			}
			this->ShiftR(this->Bit0());
		}
		else
		{
			temp->Sub(this);
			if (temp->IsZero())
			{
				break;
			}
			temp->ShiftR(temp->Bit0());
		}
	}

	// add common divisor of 2
	this->ShiftL(shift);

	// release temporary buffer
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(1);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp);
#endif
}

#else

// Euclidean GCD
void bigint::GCD(const bigint* num1, const bigint* num2)
{
	// some number is 0, result will be 1
	if (num1->IsZero() || num2->IsZero())
	{
		this->Set1();
		return;
	}

	// both numbers are small, use fast integer
	if ((num1->m_Num == 1) && (num2->m_Num == 1))
	{
		BIGINT_BASE a = num1->m_Data[0];
		BIGINT_BASE b = num2->m_Data[0];
		for (;;)
		{
			a = a % b;
			if (a == 0)
			{
				a = b;
				break;
			}

			b = b % a;
			if (b == 0) break;
		}

		this->SetSize(1);
		this->m_Data[0] = a;
		this->m_Sign = False;
		return;
	}

	// get temporary buffer
	bigint* temp = this->GetTemp();

	// copy of numbers
	temp->Copy(num2);
	this->Copy(num1);

	// absolute value
	temp->Abs();
	this->Abs();

	// search loop
	for (;;)
	{
		// get remainder Temp2 % this
		temp->Mod(this);

		// if remainder is 0, result will be 'this'
		if (temp->IsZero()) break;

		// get remainder this % Temp2
		this->Mod(temp);

		// if remainder is 0, result will be Temp2
		if (this->IsZero())
		{
			this->Copy(temp);
			break;
		}
	}

	// release temporary buffer
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(1);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp);
#endif
}

#endif

// ---------------------------------------------------------------------------
//                     multiply number * 10 and add digit
// ---------------------------------------------------------------------------

void bigint::Mul10(BIGINT_BASE carry)
{
	// expand number
	int n = this->m_Num;
	this->Resize(n+1);

	// multiply loop
	BIGINT_BASE rH, rL;
	BIGINT_BASE *d;

	// multiply loop
	d = this->m_Data;
	for (; n > 0; n--)
	{
		bigint::MulHL(&rH, &rL, *d, 10);
		rL += carry;
		if (rL < carry) rH++;
		*d = rL;
		carry = rH;
		d++;
	}
	*d = carry;

	// reduce destination
	this->Reduce();
}

// ---------------------------------------------------------------------------
//              divide number / 10 and return digit
// ---------------------------------------------------------------------------

char bigint::Div10()
{
	// divide loop
	int n = this->m_Num;

#if BIGINT_BASE_BITS >= 32
	u32* d = (u32*)&this->m_Data[n];
	u64 a, carry;
	carry = 0;
#if BIGINT_BASE_BITS == 64
	n *= 2;
#endif
	for (; n > 0; n--)
	{
		d--;
		carry = *d | (carry << 32);
		a = carry / 10;
		carry -= a*10;
		*d = (u32)a;
	}
#elif BIGINT_BASE_BITS >= 16
	u16* d = &this->m_Data[n];
	u32 a, carry;
	carry = 0;
	for (; n > 0; n--)
	{
		d--;
		carry = *d | (carry << 16);
		a = carry / 10;
		carry -= a*10;
		*d = (u16)a;
	}
#else
	u8* d = &this->m_Data[n];
	u16 a, carry;
	carry = 0;
	for (; n > 0; n--)
	{
		d--;
		carry = *d | (carry << 8);
		a = carry / 10;
		carry -= a*10;
		*d = (u8)a;
	}
#endif

	// reduce destination
	this->Reduce();

	return (char)carry;
}

// ---------------------------------------------------------------------------
//             import number from ASCIIZ text
// ---------------------------------------------------------------------------

void bigint::FromText(const char* text)
{
	// clear number
	this->Set0();

	// skip spaces
	char ch;
	for (;;)
	{
		ch = *text++;
		if (ch == 0) return;
		if ((ch < 0) || (ch > ' ')) break;
	}
	
	// get sign
	Bool sign = False;
	if (ch == '+') ch = *text++;
	if (ch == '-')
	{
		ch = *text++;
		sign = True;
	}	

	// add digits
	while ((ch >= '0') && (ch <= '9'))
	{
		this->Mul10(ch - '0');
		ch = *text++;
	}

	// set sign
	if (!this->IsZero()) m_Sign = sign;
}

// ---------------------------------------------------------------------------
//       export number to text buffer (returns number of characters)
// ---------------------------------------------------------------------------

int bigint::ToText(char* buf, int size) const
{
	// get temporary buffer
	bigint* temp = this->GetTemp();

	// backup number
	temp->Copy(this);

	// prepare pointer to end of buffer
	char* d = &buf[size];
	
	// store digits
	int n = 0;
	do {
		if (n >= size) break;
		d--;
		*d = temp->Div10() + '0';
		n++;
	} while (!temp->IsZero());

	// store sign
	if (this->IsNeg() && (n < size))
	{
		d--;
		*d = '-';
		n++;
	}

	// shift text in buffer
	if (n < size)
	{
		memmove(buf, d, n);
		buf[n] = 0;
	}

	// release temporary buffer
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	FreeTemp(1);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	FreeTemp(temp);
#endif

	return n;
}

// ---------------------------------------------------------------------------
// load table Bernoulli number - numerator (index = 0..BernMax() = 0..4096)
// ---------------------------------------------------------------------------

void bigint::BernNum(int inx)
{
	// 0 -> 1
	if (inx == 0)
		this->Set1();

	// 1 -> -1 (-1/2)
	else if(inx == 1)
		this->SetInt(-1);

	// invalid index or odd index -> 0
	else if ((inx < 0) || (inx > this->BernMax()) || ((inx & 1) == 1))
		this->Set0();

	// get table
	else
		this->Copy(&bern_num[inx/2-1]);
}

// ---------------------------------------------------------------------------
// load table Bernoulli number - denominator (index = 0..BernMax() = 0..4096)
// ---------------------------------------------------------------------------

void bigint::BernDen(int inx)
{
	// 1 -> 2 (-1/2)
	if (inx == 1)
		this->SetInt(2);

	// invalid index, 0 or odd index -> 1
	else if ((inx <= 0) || (inx > this->BernMax()) || ((inx & 1) == 1))
		this->Set1();

	// get table
	else
		this->Copy(&bern_den[inx/2-1]);
}

// ---------------------------------------------------------------------------
//     generate array of even Bernoulli numbers as fraction, direct mode
// ---------------------------------------------------------------------------
//  n = required number of Bernoulli numbers (generate even numbers B2..B2n)
//  numer = pointer to array of 'n' bigint numbers to store result numerators
//			(GetArr function can be used to allocate arrays of bigint)
//  denom = pointer to array of 'n' bigint numbers to store result denominators
//  cb = callback function to indicate progress (NULL=not used)
// Requires '4*n+2' bigint numbers to be temporary allocated using malloc function.
// Note: B0=1 (skipped), B1=-1/2 (skipped), B2=1/6, B3=0 (skipped), B4=-1/30, B5=0 (skipped), ...
// Note2: bigint::Bernoulli is faster than frac::Bernoulli

void bigint::Bernoulli(int n, bigint* numer, bigint* denom, bernoulli_cb cb /* = NULL*/)
{
	// real count of numbers
	n = n*2+1;

	// allocate buffers
	bigint* num = bigint::GetArr(n); // numerator
	bigint* den = bigint::GetArr(n); // denominator

	// total number of loops
	u64 loops = (u64)n*(n+1)/2;
	u64 loop = 0;	
	if (cb != NULL) cb(0);

	// local variables
	int j, m;//, k, k2;
	bigint tmp;
	int inx = 0; // destination index

	// first entry = 1
	num[0].Set1();
	den[0].Set1();

	// main iteration loop
	for (m = 1; m < n; m++)
	{
		// num[m] = 1; den[m] = m+1
		num[m].Set1();
		den[m].SetInt(m+1);

		for (j = m; j >= 1; j--)
		{
			// num[j-1] = num[j-1] * den[j]
			num[j-1].Mul(&den[j]);

			// num[j-1] = num[j-1] - num[j]*den[j-1]
			tmp.Mul(&num[j], &den[j-1]);
			tmp.Neg();
			num[j-1].Add(&tmp);

			// den[j-1] = den[j-1] * den[j]
			den[j-1].Mul(&den[j]);

			// num[j-1] = num[j-1] * j
			tmp.SetInt(j);
			num[j-1].Mul(&tmp);

			// divide common power of 2
/*			k = num[j-1].Bit0();
			k2 = den[j-1].Bit0();
			if (k > k2) k = k2;
			if (k > 0)
			{
				num[j-1].ShiftR(k);
				den[j-1].ShiftR(k);
			}
  */
			// divide by greatest common divisor of num[j-1] and den[j-1]
			tmp.GCD(&num[j-1], &den[j-1]);
			if (!tmp.EquInt(1))
			{
				num[j-1].Div(&tmp);
				den[j-1].Div(&tmp);
			}

			// progress
			if (cb != NULL)
			{
				loop++;
				if ((loop & 0x1ff) == 0) cb((int)(loop*1000/loops));
			}
		}

		// store result
		if ((m & 1) == 0)
		{
			numer[inx].Copy(&num[0]);
			denom[inx].Copy(&den[0]);
			inx++;
		}
	}

	if (cb != NULL) cb(1000);

	// release buffers
	bigint::FreeArr(num, n);
	bigint::FreeArr(den, n);
}

// ---------------------------------------------------------------------------
// initialize state of Bernoulli generator (n = required number of Bernoulli numbers B2..B2n)
// ---------------------------------------------------------------------------

void bigint::BernInit(int n, bern_state* state)
{
	int n2 = n*2+1; // size of temporary buffers
	state->n = n; // required number of Bernoulli numbers
	state->num = bigint::GetArr(n2); // temporary numerators
	state->den = bigint::GetArr(n2); // temporary denominators
	state->numer = bigint::GetArr(n); // result numerators
	state->denom = bigint::GetArr(n); // result denominators
	state->loop = 0; // loop counter
	state->inx = 0; // destination index
	state->loop1 = 1; // index of outer loop
	state->loop2 = 1; // index of inner loop
}

// ---------------------------------------------------------------------------
//        terminate state of Bernoulli generator (deletes buffers)
// ---------------------------------------------------------------------------

void bigint::BernTerm(bern_state* state)
{
	int n = state->n; // number of Bernoulli numbers
	int n2 = n*2+1; // size of temporary buffers
	bigint::FreeArr(state->num, n2);
	bigint::FreeArr(state->den, n2);
	bigint::FreeArr(state->numer, n);
	bigint::FreeArr(state->denom, n);
	state->n = 0;
	state->num = NULL;
	state->den = NULL;
	state->numer = NULL;
	state->denom = NULL;
}

// ---------------------------------------------------------------------------
//          upsize state of Bernoulli generator (resize buffers)
// ---------------------------------------------------------------------------

void bigint::BernUpsize(int n, bern_state* state)
{
	int nold = state->n;
	if (n <= nold) return;
	int n2 = n*2+1; // size of temporary buffers
	int n2old = nold*2+1;
	state->n = n; // required number of Bernoulli numbers
	state->num = bigint::ResizeArr(state->num, n2old, n2); // temporary numerators
	state->den = bigint::ResizeArr(state->den, n2old, n2); // temporary denominators
	state->numer = bigint::ResizeArr(state->numer, nold, n); // result numerators
	state->denom = bigint::ResizeArr(state->denom, nold, n); // result denominators
}

// ---------------------------------------------------------------------------
//    generate array of even Bernoulli numbers as fraction, using state
// ---------------------------------------------------------------------------
//  state = state of generator (initialized by bigint::BernInit od bigint::BernLoad)
//  cb = callback function to indicate progress (NULL=not used)

void bigint::Bernoulli(bern_state* state, bernoulli_cb cb /* = NULL */)
{
	// required number of Bernoulli numbers
	int n0 = state->n;

	// size of temporary buffers
	int n = n0*2+1;

	// load old state
	bigint* num = state->num; // temporary numerators
	bigint* den = state->den; // termporary denominators
	bigint* numer = state->numer; // result numerators
	bigint* denom = state->denom; // result denominators		
	u64 loop = state->loop; // current loop counter
	int inx = state->inx; // destination index
	int m = state->loop1; // index of outer loop
	int j = state->loop2; // index of inner loop

	// total number of loops
	u64 loops = (u64)n*(n+1)/2;

	// local variables
	int k, k2;
	bigint tmp;

	// first entry = 1
	if ((m == 1) && (j == 1))
	{
		num[0].Set1();
		den[0].Set1();
	}

	// outer loop
	for (; m < n; m++)
	{
		// num[m] = 1; den[m] = m+1
		num[m].Set1();
		den[m].SetInt(m+1);

		// inner loop
		for (; j >= 1; j--)
		{
			// progress
			if (cb != NULL)
			{
				loop++;
				if ((loop & 0x3ff) == 0)
				{
					state->loop = loop;
					state->inx = inx;
					state->loop1 = m;
					state->loop2 = j;
					cb((int)(loop*1000/loops));
				}
			}

			// num[j-1] = num[j-1] * den[j]
			num[j-1].Mul(&den[j]);

			// num[j-1] = num[j-1] - num[j]*den[j-1]
			tmp.Mul(&num[j], &den[j-1]);
			tmp.Neg();
			num[j-1].Add(&tmp);

			// den[j-1] = den[j-1] * den[j]
			den[j-1].Mul(&den[j]);

			// num[j-1] = num[j-1] * j
			tmp.SetInt(j);
			num[j-1].Mul(&tmp);

			// divide common power of 2
			k = num[j-1].Bit0();
			k2 = den[j-1].Bit0();
			if (k > k2) k = k2;
			if (k > 0)
			{
				num[j-1].ShiftR(k);
				den[j-1].ShiftR(k);
			}
 
			// divide by greatest common divisor of num[j-1] and den[j-1]
			tmp.GCD(&num[j-1], &den[j-1]);
			num[j-1].Div(&tmp);
			den[j-1].Div(&tmp);
		}

		// store result
		if ((m & 1) == 0)
		{
			numer[inx].Copy(&num[0]);
			denom[inx].Copy(&den[0]);
			inx++;
		}

		// restart inner loop
		j = m + 1;
	}

	state->loop = loop;
	state->inx = inx;
	state->loop1 = m;
	state->loop2 = j;
}

// ---------------------------------------------------------------------------
//                save number to the file (with CRC32 update)
// ---------------------------------------------------------------------------

u32 bigint::Save(FILE* f, u32 crc) const
{
	// save file header
	bigint_file h;
	size_t n = m_Num*BIGINT_BASE_BYTES;
	h.size = n;
	if (m_Sign) h.size = - h.size;
	crc = Crc32_Buf(crc, &h.size, sizeof(s64));
	size_t s = fwrite(&h.size, 1, sizeof(s64), f);
	if (s != sizeof(s64)) Fatal("Save: File write error");

	// save data
	crc = Crc32_Buf(crc, m_Data, (int)n);
	s = fwrite(m_Data, 1, n, f);
	if (s != n) Fatal("Save: File write error");
	return crc;
}

// ---------------------------------------------------------------------------
//              load number from the file (with CRC32 update)
// ---------------------------------------------------------------------------

u32 bigint::Load(FILE* f, u32 crc)
{
	// load file header
	bigint_file h;
	size_t s = fread(&h.size, 1, sizeof(s64), f);
	if (s != sizeof(s64)) Fatal("Load: File read error");
	crc = Crc32_Buf(crc, &h.size, sizeof(s64));
	m_Sign = False;
	if (h.size < 0)
	{
		m_Sign = True;
		h.size = - h.size;
	}

	// prepare data size
	size_t n = (size_t)h.size;
	int num = (int)((n + BIGINT_BASE_BYTES - 1)/BIGINT_BASE_BYTES);
	if (num == 0)
	{
		this->Set0();
		return crc;
	}

	// load data
	this->SetSize(num);
	m_Data[num-1] = 0;
	s = fread(m_Data, 1, n, f);
	if (s != n) Fatal("Load: File read error");
	crc = Crc32_Buf(crc, m_Data, (int)n);
	return crc;
}

// ---------------------------------------------------------------------------
//                 save state of Bernoulli generator
// ---------------------------------------------------------------------------
//	f = file
//  state = state of generator

void bigint::BernSave(FILE* f, const bern_state* state)
{
	// prepare file header
	bern_file h;
	h.magic = BERN_MAGIC;
	h.loop = state->loop;
	h.inx = state->inx;
	h.loop1 = state->loop1;
	h.loop2 = state->loop2;

	// save header
	u32 crc = Crc32(&h, sizeof(h));
	size_t s = fwrite(&h, 1, sizeof(h), f);
	if (s != sizeof(h)) Fatal("BernSave: File write error");

	// save temporary numerators
	int n = state->loop1;
	int i;
	for (i = 0; i < n; i++) crc = state->num[i].Save(f, crc);

	// save temporary denominators
	for (i = 0; i < n; i++) crc = state->den[i].Save(f, crc);

	// save output numerators
	n = state->inx;
	for (i = 0; i < n; i++) crc = state->numer[i].Save(f, crc);

	// save output denominators
	for (i = 0; i < n; i++) crc = state->denom[i].Save(f, crc);

	// save CRC32
	s = fwrite(&crc, 1, sizeof(u32), f);
	if (s != sizeof(u32)) Fatal("BernSave: File write error");
}

void bigint::BernSaveFile(const char* filename, const bern_state* state)
{
	// open file
	FILE* f = fopen(filename, "wb");
	if (f == NULL) Fatal("BernSaveFile: File write error");

	// write state
	bigint::BernSave(f, state);

	// close file
	fclose(f);
}

// ---------------------------------------------------------------------------
// load state of Bernoulli generator (returns False if file not found or if has incorrect magic)
// ---------------------------------------------------------------------------
//	f = file
//  state = state of generator

void bigint::BernLoad(FILE* f, bern_state* state)
{
	// load header
	bern_file h;
	size_t s = fread(&h, 1, sizeof(h), f);
	if (s != sizeof(h)) Fatal("BernLoad: File read error");
	u32 crc = Crc32(&h, sizeof(h));

	// check header
	if (h.magic != BERN_MAGIC) Fatal("BernLoad: File read error - incorrect file");

	// get file header
	state->loop = h.loop;
	state->inx = h.inx;
	int n = h.loop1;
	state->loop1 = n;
	state->loop2 = h.loop2;
	int n0 = n/2;
	state->n = n0;
	if (h.inx > (u32)n0) Fatal("BernLoad: File read error - corrupted file");

	// create buffers
	n = n0*2+1;
	state->num = bigint::GetArr(n); // temporary numerators
	state->den = bigint::GetArr(n); // temporary denominators
	state->numer = bigint::GetArr(n0); // result numerators
	state->denom = bigint::GetArr(n0); // result denominators

	// load temporary numerators
	n = h.loop1;
	int i;
	for (i = 0; i < n; i++) crc = state->num[i].Load(f, crc);

	// load temporary denominators
	for (i = 0; i < n; i++) crc = state->den[i].Load(f, crc);

	// load output numerators
	n = h.inx;
	for (i = 0; i < n; i++) crc = state->numer[i].Load(f, crc);

	// load output denominators
	for (i = 0; i < n; i++) crc = state->denom[i].Load(f, crc);

	// check CRC
	u32 crc2 = ~crc;
	s = fread(&crc2, 1, sizeof(u32), f);
	if (s != sizeof(u32)) Fatal("BernLoad: File read error");
	if (crc2 != crc) Fatal("BernLoad: File read error - checksum error");
}

Bool bigint::BernLoadFile(const char* filename, bern_state* state)
{
	// open file
	FILE* f = fopen(filename, "rb");
	if (f == NULL) return False;

	// write state
	bigint::BernLoad(f, state);

	// close file
	fclose(f);

	return True;
}
