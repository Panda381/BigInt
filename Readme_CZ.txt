BigInt Bernoulli - knihovna pro velká celá èísla s generátorem Bernoulliho èísel
Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
Datum verze: 5.3.2023

Stránka projektu: https://www.breatharian.eu/sw/bigint
Projekt na GitHub: https://github.com/Panda381/BigInt


Knihovna BigInt slouží k práci s velkými celými èísly. Knihovna
je doplnìna generátorem Bernoulliho èísel, která lze použít napø.
k výpoètu velkých lineárních faktoriálù. Knihovna obsahuje
pøedgenerovanou databázi 5000 sudých Bernoulliho èísel (B2 až
do B10000). Knihovna je souèástí knihovny pro floating-point
výpoèty s vysokou pøesností na Raspberry Pico.


Použití
-------
Program se spouští se 3 parametry v pøíkazovém øádku. Prvním
parametrem je poèet generovaných Bernoulliho èísel (napø. èíslo
5000 vygeneruje èísla B2 až B10000). Druhým parametrem je typ
výstupního souboru: 1=CSV s tabulátory jako oddìlovaè, 2=CSV
s èárkami, 3=CSV se støedníky, 4=zdrojový kód CPP. Tøetím
parametrem je jméno výstupního souboru.

Soubor !gener.bat je povelový soubor slouží jako ukázka použití
- vygeneruje všechny typy souborù pro 5000 èísel B2 až B10000
(soubory bern_com.csv, bern_const.cpp, bern_sem.csv a
bern_tab.csv).

Soubor Bernoulli.bin je cache obsahující databázi vygenerovaných
Bernoulliho èísel. Výpoèet Bernoulliho èísel je zdlouhavá operace
(výpoèet 5000 èísel zabere nìkolik týdnù), proto je dosažený stav
bìhem výpoètu prùbìžnì ukládán do souboru Bernoulli.bin. Výpoèet
tak lze kdykoliv pøerušit a opìt znovu spustit s pokraèováním
od stejného místa. Jsou-li požadovaná Bernoulliho èísla, která
se už nacházejí v databázi (tj. do poètu 5000), použijí se hodnoty
s databáze a jejich vygenerování je podstatnì rychlejší (zápis
5000 Bernoulliho èísel z cache na disk zabere nìkolik minut).

Složka bernoulli-mini obsahuje zjednodušenou verzi knihovny,
psanou v C kódu. Knihovnu lze pøeložit jako program MS VC++
2005 nebo zaèlenit do projektu Raspberry Pico s ARM-GCC.
Knihovna vygeneruje prvních 512 Bernoulliho èísel (B2 až B1024)
v C kódu (bernoulli.c a bernoulli.csv) a neumožòuje cachování
výpoètu.


Poznámky
--------
Dva identicky generované soubory Bernoulli.bin se mohou
lišit na offsetu 5 - položka "loop", která nemusí být
správnì obnovena pøi pøerušení a pokraèování generování
(pro správnou funkènost není dùležité).

Program je vytvoøen v Microsoft Visual Studio VC++ 2005.
Program je kompilován v 64-bitovém módu x64. Je možná
kompilace i pro 32-bitový mód, je však znaènì pomalejší
a mùže trpìt nedostatkem pamìti.

Knihovna obsahuje èásti kódu optimalizované assemblerem
x64, které jsou pøekládány programem NASM 2.13.

Knihovna vznikla k výpoètu Bernoulliho èísel pro úèely
knihovny pro výpoèet èísel s plovoucí desetinnou èárkou
s vysokou pøesností, konkrétnì pro výpoèet velkých
lineárních faktoriálù. Prvních 512 Bernoulliho èísel
(B2 až B1024) umožòuje výpoèet lineárních faktoriálù
s pøesností èísel 4096 bitù, tj. 1224 èíslic. Plný
rozsah 5000 Bernoulliho èísel (B2 až B10000) by bylo
možné použít pro výpoèty velkých lineárních faktoriálù
s pøesností 40000 bitù, tj. 12240 èíslic.

Nejvyšší èíslo B10000 má èitatel dlouhý 27691 èíslic
(jmenovatel je 13 èíslic) a zabírá 11504 bajtù.
