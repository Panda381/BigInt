
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
// main.cpp - main code

#include "../include.h"

#define BUF_SIZE 1000000
char EditBuf[BUF_SIZE];
char EditBuf2[BUF_SIZE];

// output format
#define FORM_NO		0	// no output file
#define FORM_CSVTAB	1	// *.csv Excel tabulator format
#define FORM_CSVCOM	2	// *.csv Excel comma ',' format
#define FORM_CSVSEM	3	// *.csv Excel semicolon ';' format
#define FORM_CPP	4	// *.cpp C++ format
#define FORM_MAX	4	// max. format
int Format;

#ifdef ASM64
extern "C" u64 CheckComp_x64(u64 par1, u64 par2, u64 par3, u64 par4, u64 par5, u64 par6);
#endif // ASM64

// fatal error
void Fatal(const char* txt)
{
	printf("%s\n", txt);
	exit(1);
}

// current state
bern_state BernState;
int BernMaxSaved;
int BernStateCnt = 0;
int BernNum; // required number of Bernoulli numbers
const char* OutFileName;
time_t LastSaveTime; // last save time

// save state
void BernSave()
{
	// save cache file to temporary file
	bigint::BernSaveFile(TMPFILE, &BernState);

	// delete output file
	unlink(BINFILE);

	// rename file
	if (rename(TMPFILE, BINFILE)) Fatal("File write error - rename");
}

// Bernoulli progress
void BernProg(int permille)
{
	time_t t = ::time(NULL);
	int dif = (int)(t - LastSaveTime);
	if (dif >= 60)
	{
		printf("\rSaving cache B2..B%d...        ", BernState.inx*2);
		BernSave();
		BernMaxSaved = BernState.inx*2;
		LastSaveTime = t;
	}

	// print progress
	printf("\rBernoulli: %d.%d%% (B%d, saved B%d) ", permille/10, permille % 10, BernState.inx*2, BernMaxSaved);
}

// export Bernoulli numbers to csv format
void BernCsv(char ch)
{
	FILE* f = fopen(OutFileName, "w");
	if (f == NULL) Fatal("Error opening output file");

	// write numerators - data
	int i;
	for (i = 0; i < BernNum; i++)
	{
		BernState.numer[i].ToText(EditBuf, BUF_SIZE);
		BernState.denom[i].ToText(EditBuf2, BUF_SIZE);
		fprintf(f, "%d%c%s%c%s\n", i*2+2, ch, EditBuf, ch, EditBuf2);
	}

	fclose(f);
}

// export Bernoulli numbers to C++ format
void BernCpp()
{
	FILE* f = fopen(OutFileName, "w");
	if (f == NULL) Fatal("Error opening output file");

	fprintf(f, "\n// ****************************************************************************\n");
	fprintf(f, "//\n");
	fprintf(f, "//                           Bernoulli numbers\n");
	fprintf(f, "//\n");
	fprintf(f, "// ****************************************************************************\n");
	fprintf(f, "//#define BIGINT_BERN_NUM %d	// number of table Bernoulli numbers (only even numbers B2, B4,..)\n", BernNum);
	fprintf(f, "\n");
	fprintf(f, "//typedef unsigned long long u64;\n");
	fprintf(f, "//typedef unsigned char Bool;\n");
	fprintf(f, "\n");
	fprintf(f, "//Big integer - constant\n");
	fprintf(f, "//typedef struct\n");
	fprintf(f, "//{\n");
	fprintf(f, "//\tconst u64*\tm_Data;\t// array of segments (number is always positive)\n");
	fprintf(f, "//\tint\t\t\tm_Num;\t// number of segments u64 (0=zero number)\n");
	fprintf(f, "//\tBool\t\tm_Sign;\t// sign flag\n");
	fprintf(f, "//} cbigint;\n");
	fprintf(f, "\n");
	fprintf(f, "#include \"../include.h\"\n");

	// write numerators - data
	fprintf(f, "\n// Bernoulli numbers - numerators, data\n");
	int i, j, n;
	for (i = 0; i < BernNum; i++)
	{
		n = BernState.numer[i].m_Num;
		fprintf(f, "const u64 bern_num_data%d[%d] = { ", i, n);
		for (j = 0; j < n-1; j++) fprintf(f, "0x%llxULL, ", BernState.numer[i].m_Data[j]);
		fprintf(f, "0x%llxULL };\n", BernState.numer[i].m_Data[j]);
	}

	// write numerators - headers
	fprintf(f, "\n// Bernoulli numbers - numerators, headers\n");
	fprintf(f, "const cbigint bern_num[BIGINT_BERN_NUM] = { // BIGINT_BERN_NUM=%d\n", BernNum);
	for (i = 0; i < BernNum; i++)
	{
		n = BernState.numer[i].m_Num;
		BernState.numer[i].ToText(EditBuf, BUF_SIZE);
		fprintf(f, "\t{ bern_num_data%d, %d, %d },\t// B%d: %s\n", i, n, BernState.numer[i].m_Sign, i*2+2, EditBuf);
	}
	fprintf(f, "};\n");

	// write denominators - data
	fprintf(f, "\n// Bernoulli numbers - denominators, data\n");
	for (i = 0; i < BernNum; i++)
	{
		n = BernState.denom[i].m_Num;
		fprintf(f, "const u64 bern_den_data%d[%d] = { ", i, n);
		for (j = 0; j < n-1; j++) fprintf(f, "0x%llxULL, ", BernState.denom[i].m_Data[j]);
		fprintf(f, "0x%llxULL };\n", BernState.denom[i].m_Data[j]);
	}

	// write denominators - headers
	fprintf(f, "\n// Bernoulli numbers - denominators, headers\n");
	fprintf(f, "const cbigint bern_den[BIGINT_BERN_NUM] = { // BIGINT_BERN_NUM=%d\n", BernNum);
	for (i = 0; i < BernNum; i++)
	{
		n = BernState.denom[i].m_Num;
		BernState.denom[i].ToText(EditBuf, BUF_SIZE);
		fprintf(f, "\t{ bern_den_data%d, %d, %d },\t// B%d: %s\n", i, n, BernState.denom[i].m_Sign, i*2+2, EditBuf);
	}
	fprintf(f, "};\n");

	fclose(f);
}

int main(int argc, char* argv[])
{
#ifndef WIN64
#error "Must be compiled as 64-bit!"
#endif

#ifdef ASM64
	// check X64 compiler compatibility
	if (CheckComp_x64(123, 456, 789, 321, 654, 987) != 1)
	{
		printf("Compiler error CheckComp_x64\n");
		return 1;
	}
#endif // ASM64

	// check checksum
	if (!CrcCheck())
	{
		printf("CRC32 error\n");
		return 1;
	}

	// load output format
	Format = FORM_NO;
	if (argc >= 3) Format = atoi(argv[2]);

	// check command line arguments
	if ((argc < 3) || (argc > 4) || (Format < 0) || (Format > FORM_MAX) || ((argc == 3) && (Format != FORM_NO)))
	{
		printf("Syntax: Bernoulli num format file\n"
				"     num ... number of Bernoulli numbers to generate\n"
				"     format ... format of output file:\n"
				"              0 = no output file, only generate numbers\n"
				"              1 = *.csv Excel tabulator format\n"
				"              2 = *.csv Excel comma ',' format\n"
				"              3 = *.csv Excel semicolon ';' format\n"
				"              4 = *.cpp C++ format\n"
				"     file ... output file (not needed if format=0)\n");
		return 1;
	}

	// get command line arguments
	BernNum = atoi(argv[1]);
	if (BernNum < 1) BernNum = 1;
	OutFileName = argv[3];
	BernMaxSaved = 0;
	printf("Generating %d Bernoulli numbers (B2..B%d) to the file %s\n", BernNum, BernNum*2, OutFileName);

	// load old state (or create empty state)
	Bool res = bigint::BernLoadFile(BINFILE, &BernState);
	if (!res) bigint::BernLoadFile(TMPFILE, &BernState);
	if (res && (BernState.inx > 0)) printf("Cache loaded, available %d Bernoulli numbers (B2..B%d)\n", BernState.inx, BernState.inx*2);
	if (!res) bigint::BernInit(BernNum, &BernState);
	BernMaxSaved = BernState.inx*2;

	// upsize buffers
	if (BernNum > BernState.n) bigint::BernUpsize(BernNum, &BernState);

	// generate new Bernoulli numbers
	if (BernNum > BernState.inx)
	{
		// last save time
		LastSaveTime = ::time(NULL);

		// generate
		bigint::Bernoulli(&BernState, BernProg);
		printf("\r                                              \r");

		// save cache file
		printf("Saving cache with %d Bernoulli numbers\n", BernState.inx);
		BernSave();
	}

	// export Bernoulli numbers
	if (Format == FORM_CPP) BernCpp(); // C++ format
	if (Format == FORM_CSVTAB) BernCsv('\t'); // csv tabulator
	if (Format == FORM_CSVCOM) BernCsv(','); // csv comma
	if (Format == FORM_CSVSEM) BernCsv(';'); // csv semicolon

	return 0;
}
