
// ****************************************************************************
//                                 
//                            Big integers
//
// ****************************************************************************
// BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
// bigint.h - big-integers library headers

#ifdef WIN64
#define BIGINT_BASE_BITS	64			// number of bits of base segment

#else
#define BIGINT_BASE_BITS	32			// number of bits of base segment
//#define BIGINT_BASE_BITS	16			// number of bits of base segment
//#define BIGINT_BASE_BITS	8			// number of bits of base segment

#endif

#define BIGINT_BERN_NUM 2048	// number of table Bernoulli numbers (only even numbers B2, B4,..)

// config
#if BIGINT_BASE_BITS == 64
#define BIGINT_BASE		u64			// type of base segment unsigned
#define BIGINT_BASES	s64			// type of base segment signed

#elif BIGINT_BASE_BITS == 32
#define BIGINT_BASE		u32			// type of base segment unsigned
#define BIGINT_BASES	s32			// type of base segment signed

#elif BIGINT_BASE_BITS == 16
#define BIGINT_BASE		u16			// type of base segment unsigned
#define BIGINT_BASES	s16			// type of base segment signed

#elif BIGINT_BASE_BITS == 8
#define BIGINT_BASE		u8			// type of base segment unsigned
#define BIGINT_BASES	s8			// type of base segment signed

#else
#error "Invalid BIGINT_BASE_BITS!"
#endif

#define BIGINT_BASE_BYTES (BIGINT_BASE_BITS/8)	// number of bytes per base segment
#define BIGINT_BASE_LAST	((BIGINT_BASE)1 << (BIGINT_BASE_BITS-1))	// last bit in segment

// x64 string functions
#ifdef ASM64
extern "C" int ShiftL1Str_x64(int num, u64* src, u64 carry); // num = number of u64 segments, src = start of string, carry = 0 or 1
extern "C" int ShiftR1Str_x64(int num, u64* src, u64 carry); // num = number of u64 segments, src = after end of string, carry = 0 or 1
extern "C" void ShiftLStr_x64(int num, int shift, u64* dst, u64* src); // num = transfer segments, shift = shift, dst = after destination, src = after source
extern "C" void ShiftRStr_x64(int num, int shift, u64* dst, u64* src); // num = transfer segments, shift = shift, dst = after destination, src = after source
extern "C" int AddStr_x64(int num1, int num2, u64* dst, const u64* src1, const u64* src2); // num1,num2 = number of u64 segments of src1,src2
extern "C" int SubStr_x64(int num1, int num2, u64* dst, const u64* src1, const u64* src2); // num1,num2 = number of u64 segments of src1,src2
extern "C" int NegStr_x64(int num, u64* src); // returns 1 if result is not 0
extern "C" void MulSetStr_x64(int num, u64 a, u64* dst, const u64* src); // stores carry to dst[num]
extern "C" void MulAddStr_x64(int num, u64 a, u64* dst, const u64* src); // stores carry to dst[num]
extern "C" u64 DivStr_x64(int num, u64 a, u64* dst, const u64* src); // returns remainder, dst/src = after end of string
extern "C" void DivSubStr_x64(int num, u64* dst, const u64* src); // num = number of u64 segments, src = start of string
extern "C" u64 ModStr_x64(int num, u64 a, const u64* src); // returns remainder, src = after end of string
#endif

// Big integer - constant
typedef struct
{
	const u64*		m_Data;		// array of segments (number is always positive)
	int				m_Num;		// number of segments u64 (0=zero number)
	Bool			m_Sign;		// sign flag
} cbigint;

// Bernoulli numbers - numerators
extern const cbigint bern_num[BIGINT_BERN_NUM];

// Bernoulli numbers - denominators
extern const cbigint bern_den[BIGINT_BERN_NUM];

class bigint;

// state of Bernoulli generator
typedef struct {
	int			n;		// required number of numbers (determines size of buffers)
	bigint*		num;	// array of temporary numerators, 'n*2+1' entries
	bigint*		den;	// array of temporary denominators, 'n*2+1' entries
	bigint*		numer;	// array of result numerators, 'n' entries
	bigint*		denom;	// array of result denominators, 'n' entries
	u64			loop;	// current loop counter
	int			inx;	// destination index into result array (= number of completed result numbers)
	int			loop1;	// current index of outer loop (= number of completed temporary numbers)
	int			loop2;	// current index of inner loop (= current processed temporary number)
} bern_state;

#define BERN_MAGIC 0xBEFEED64	// file header magic ("Bernoulli Feed 64-bit")

// Bernoulli state file header
#pragma pack(push, 1)	// strict structure
typedef struct
{
	u32			magic;	// magic BERN_MAGIC
	u64			loop;	// current loop counter
	u32			inx;	// destination index (= number of completed result numbers)
	u32			loop1;	// current index of outer loop (= number of completed temporary numbers)
	u32			loop2;	// current index of inner loop (= current processed temporary number)
						// - follow data of array of temporary numerators ('loop1' entries)
						// - follow data of array of temporary denominators ('loop1' entries)
						// - follow data of array of result numerators ('inx' entries)
						// - follow data of array of result denominators ('inx' entries)
						// - CRC32 of whole file
} bern_file;
#pragma pack(pop)

// number file header
#pragma pack(push, 1)	// strict structure
typedef struct
{
	s64			size;	// number size in bytes (negative = number is negative)
						// - numeric data follow
} bigint_file;
#pragma pack(pop)

// Big integer
class bigint
{
public:

	BIGINT_BASE*	m_Data;		// array of segments (number is always positive)
	int				m_Num;		// number of valid segments (0=zero number)
	int				m_Max;		// maximum allocated segments
	Bool			m_Sign;		// sign flag

	// get temporary number
	static bigint* GetTemp();

	// free temporary number(s)
#if BIGINT_TEMPMODE == 0 // mode of temporary variables, 0=use global variables
	static void FreeTemp(int num);
#elif BIGINT_TEMPMODE == 1 // mode of temporary variables, 1=use malloc
	static void FreeTemp(void* temp);
#endif

	// constructor
	bigint();

	// destructor
	~bigint();

	// allocate array of bigint numbers
	static bigint* GetArr(int num);

	// release array of bigint numbers
	static void FreeArr(bigint* arr, int num);

	// resize array of bigint numbers (returns new address of array)
	static bigint* ResizeArr(bigint* arr, int oldnum, int newnum);

	// set number of segments, segments can stay uninitialized
	void SetSize(int num);

	// resize number of segments
	void Resize(int num);

	// reduce - delete zero segments
	void Reduce();

	// long multiply rH:rL <- a*b
#if !defined(ASM64)
	static void MulHL(BIGINT_BASE* rH, BIGINT_BASE* rL, BIGINT_BASE a, BIGINT_BASE b);
#else
	inline static void MulHL(BIGINT_BASE* rH, BIGINT_BASE* rL, BIGINT_BASE a, BIGINT_BASE b)
	{
#if BIGINT_BASE_BITS == 8 // u8
		MulHL8(rH, rL, a, b);
#elif BIGINT_BASE_BITS == 16 // u16
		MulHL16(rH, rL, a, b);
#elif BIGINT_BASE_BITS == 32 // u32
		MulHL32(rH, rL, a, b);
#else // u64
		MulHL64(rH, rL, a, b);
#endif
	}
#endif

	// copy from another number
	void Copy(const bigint* num);
	void Copy(const cbigint* num);

	// exchange numbers
	void Exch(bigint* num);

	// compare numbers (result: 1 if num1 > num2, 0 if num1 == num2, -1 if num1 < num2)
	static int Comp(const bigint* num1, const bigint* num2);
	inline int Comp(const bigint* num) const { return this->Comp(this, num); }

	// compare absolute value of numbers (result: 1 if num1 > num2, 0 if num1 == num2, -1 if num1 < num2)
	static int CompAbs(const bigint* num1, const bigint* num2);
	inline int CompAbs(const bigint* num) const { return this->CompAbs(this, num); }

	// get bit length of the number (= 1 + bit position of highest '1')
	int BitLen() const;

	// get number of lower bits '0' (trailing zeros)
	int Bit0() const;

	// shift number 1 bit left (= multiply * 2)
	void ShiftL1();

	// shift number 1 bit right (= divide / 2)
	void ShiftR1();

	// shift number more bits left
	void ShiftL(int shift);

	// shift number more bits right
	void ShiftR(int shift);

	// check if number is zero
	inline Bool IsZero() const { return m_Num == 0; }

	// check if number is negative
	inline Bool IsNeg() const { return m_Sign; }

	// negate number
	inline void Neg() { if (m_Num > 0) m_Sign = !m_Sign; }

	// absolute value
	inline void Abs() { m_Sign = False; }

	// check if number is equal integer value
	Bool EquInt(BIGINT_BASES num);

	// set value 0
	void Set0();

	// set value 1
	void Set1();

	// set integer value
	void SetInt(BIGINT_BASES num);

	// add/sub two numbers (this = num1 +- num2, operands and destination can be the same)
	void AddSub(const bigint* num1, const bigint* num2, Bool sub);

	// add two numbers (this = num1 + num2, operands and destination can be the same)
	inline void Add(const bigint* num1, const bigint* num2) { this->AddSub(num1, num2, False); }
	inline void Add(const bigint* num) { this->AddSub(this, num, False); }

	// subtract two numbers (this = num1 - num2, operands and destination can be the same)
	inline void Sub(const bigint* num1, const bigint* num2) { this->AddSub(num1, num2, True); }
	inline void Sub(const bigint* num) { this->AddSub(this, num, True); }
	inline void InvSub(const bigint* num) { this->AddSub(num, this, True); }

	// multiply two numbers (this = num1 * num2, operands and destintion can be the same)
	void Mul(const bigint* num1, const bigint* num2);
	inline void Mul(const bigint* num) { this->Mul(this, num); }

	// divide two numbers (this = num1 / num2, rem = remainder or NULL if not required)
	void Div(const bigint* num1, const bigint* num2, bigint* rem = NULL);
	inline void Div(const bigint* num) { this->Div(this, num, NULL); }

	// get modulo - remainder (this = this % num, remainder will always be >= 0)
	void Mod(const bigint* num);

	// find greatest common divisor GCD
	void GCD(const bigint* num1, const bigint* num2);

	// multiply number * 10 and add digit
	void Mul10(BIGINT_BASE carry);

	// divide number / 10 and return digit
	char Div10();

	// import number from ASCIIZ text
	void FromText(const char* text);

	// export number to text buffer (returns number of characters)
	int ToText(char* buf, int size) const;

	// get max. index of table Bernoulli number (=4096)
	inline int BernMax() { return BIGINT_BERN_NUM*2; }

	// load table Bernoulli number - numerator (index = 0..BernMax() = 0..4096)
	void BernNum(int inx);

	// load table Bernoulli number - denominator (index = 0..BernMax() = 0..4096)
	void BernDen(int inx);

	// typedef callback function to indicate Bernoulli progress, in 0..1000 per mille)
typedef void (bernoulli_cb)(int permille);

	// generate array of even Bernoulli numbers as fraction, direct mode
	//  n = required number of Bernoulli numbers (generate even numbers B2..B2n)
	//  numer = pointer to array of 'n' bigint numbers to store result numerators
	//			(GetArr function can be used to allocate arrays of bigint)
	//  denom = pointer to array of 'n' bigint numbers to store result denominators
	//  cb = callback function to indicate progress (NULL=not used)
	// Requires '4*n+2' bigint numbers to be temporary allocated using malloc function.
	// Note: B0=1 (skipped), B1=-1/2 (skipped), B2=1/6, B3=0 (skipped), B4=-1/30, B5=0 (skipped), ...
	// Note2: bigint::Bernoulli is faster than frac::Bernoulli
	static void Bernoulli(int n, bigint* numer, bigint* denom, bernoulli_cb cb = NULL);

	// initialize state of Bernoulli generator (n = required number of Bernoulli numbers B2..B2n)
	static void BernInit(int n, bern_state* state);

	// terminate state of Bernoulli generator (deletes buffers)
	static void BernTerm(bern_state* state);

	// upsize state of Bernoulli generator (resize buffers)
	static void BernUpsize(int n, bern_state* state);

	// generate array of even Bernoulli numbers as fraction, using state
	//  state = state of generator (initialized by bigint::BernInit od bigint::BernLoad)
	//  cb = callback function to indicate progress (NULL=not used)
	static void Bernoulli(bern_state* state, bernoulli_cb cb = NULL);

	// save number to the file (with CRC32 update)
	u32 Save(FILE* f, u32 crc) const;

	// load number from the file (with CRC32 update)
	u32 Load(FILE* f, u32 crc);

	// save state of Bernoulli generator
	//	f = file
	//  state = state of generator
	static void BernSave(FILE* f, const bern_state* state);
	static void BernSaveFile(const char* filename, const bern_state* state);

	// load state of Bernoulli generator (returns False if file not found)
	//  n = required number of Bernoulli numbers B2..B2n
	//	f = file
	//  state = state of generator
	static void BernLoad(FILE* f, bern_state* state);
	static Bool BernLoadFile(const char* filename, bern_state* state);
	
};
