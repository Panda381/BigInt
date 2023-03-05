BigInt Bernoulli - Big Integers Library with Bernoulli Number Generator
Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
Version date: 03/05/2023

Project website: https://www.breatharian.eu/sw/bigint/index_en.html
Project on GitHub: https://github.com/Panda381/BigInt


The BigInt library is used to work with large integers. The library
is supplemented by a Bernoulli number generator, which can be used,
for example, to calculate large linear factorials. The library
contains pre-generated database of 5000 even Bernoulli numbers (B2
up to B10000). The library is part of the floating-point library
calculations with high precision on the Raspberry Pico.


Usage
-----
The program is run with 3 parameters in the command line. The first
parameter is the number of Bernoulli numbers to be generated (e.g.
the number 5000 will generate the numbers B2 to B10000). The second
parameter is the type of the output file: 1=CSV with tabs as separator,
2=CSV with commas, 3=CSV with semicolons, 4=CPP source code. The third
parameter is the name of the output file.

The !gener.bat file is a command file used as a usage example - it
generates all file types for 5000 numbers B2 to B10000 (files
bern_com.csv, bern_const.cpp, bern_sem.csv and bern_tab.csv).

The Bernoulli.bin file is a cache containing a database of generated
Bernoulli numbers. The calculation of Bernoulli numbers is a lengthy
operation (it takes several weeks to calculate 5000 numbers), so the
state obtained during the calculation is continuously stored in the
Bernoulli.bin file. The calculation can thus be interrupted at any
time and restarted again, continuing from the same point. If the
required Bernoulli numbers are already in the database (i.e. up to
5000), the values from the database are used and their generation is
much faster (writing 5000 Bernoulli numbers from the cache to disk
takes a few minutes).

The bernoulli-mini folder contains a simplified version of the library,
written in C code. The library can be compiled as an MS VC++ program
2005 or can be integrated into a Raspberry Pico project with ARM-GCC.
The library generates the first 512 Bernoulli numbers (B2 to B1024)
in C code (bernoulli.c and bernoulli.csv) and does not support caching
of calculation.


Notes
-----
Two identically generated Bernoulli.bin files may differ
at offset 5 - the "loop" entry, which is not correctly
restored when interrupting and resuming generation
(not important for proper functionality).

The program is created in Microsoft Visual Studio VC++ 2005.
The program is compiled in 64-bit x64 mode. It is also possible
to compile for 32-bit mode, but it is considerably slower and
may suffer from lack of memory.

The library contains parts of code optimized by the assembler
x64 that are compiled by NASM 2.13.

The library was created to calculate Bernoulli numbers for the purpose
of library for calculating floating point numbers with high precision,
specifically for calculating large linear factorials. The first 512
Bernoulli numbers (B2 to B1024) allow the calculation of linear
factorials with a numerical precision of 4096 bits, i.e. 1224 digits.
Full range of 5000 Bernoulli numbers (B2 to B10000) could be used to
calculate large linear factorials with an accuracy of 40000 bits, i.e.
12240 digits.

The highest number B10000 has a numerator 27691 digits long
(denominator is 13 digits) and occupies 11504 bytes.
