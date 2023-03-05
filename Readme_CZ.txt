BigInt Bernoulli - knihovna pro velk� cel� ��sla s gener�torem Bernoulliho ��sel
Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz
Datum verze: 5.3.2023

Str�nka projektu: https://www.breatharian.eu/sw/bigint
Projekt na GitHub: https://github.com/Panda381/BigInt


Knihovna BigInt slou�� k pr�ci s velk�mi cel�mi ��sly. Knihovna
je dopln�na gener�torem Bernoulliho ��sel, kter� lze pou��t nap�.
k v�po�tu velk�ch line�rn�ch faktori�l�. Knihovna obsahuje
p�edgenerovanou datab�zi 5000 sud�ch Bernoulliho ��sel (B2 a�
do B10000). Knihovna je sou��st� knihovny pro floating-point
v�po�ty s vysokou p�esnost� na Raspberry Pico.


Pou�it�
-------
Program se spou�t� se 3 parametry v p��kazov�m ��dku. Prvn�m
parametrem je po�et generovan�ch Bernoulliho ��sel (nap�. ��slo
5000 vygeneruje ��sla B2 a� B10000). Druh�m parametrem je typ
v�stupn�ho souboru: 1=CSV s tabul�tory jako odd�lova�, 2=CSV
s ��rkami, 3=CSV se st�edn�ky, 4=zdrojov� k�d CPP. T�et�m
parametrem je jm�no v�stupn�ho souboru.

Soubor !gener.bat je povelov� soubor slou�� jako uk�zka pou�it�
- vygeneruje v�echny typy soubor� pro 5000 ��sel B2 a� B10000
(soubory bern_com.csv, bern_const.cpp, bern_sem.csv a
bern_tab.csv).

Soubor Bernoulli.bin je cache obsahuj�c� datab�zi vygenerovan�ch
Bernoulliho ��sel. V�po�et Bernoulliho ��sel je zdlouhav� operace
(v�po�et 5000 ��sel zabere n�kolik t�dn�), proto je dosa�en� stav
b�hem v�po�tu pr�b�n� ukl�d�n do souboru Bernoulli.bin. V�po�et
tak lze kdykoliv p�eru�it a op�t znovu spustit s pokra�ov�n�m
od stejn�ho m�sta. Jsou-li po�adovan� Bernoulliho ��sla, kter�
se u� nach�zej� v datab�zi (tj. do po�tu 5000), pou�ij� se hodnoty
s datab�ze a jejich vygenerov�n� je podstatn� rychlej�� (z�pis
5000 Bernoulliho ��sel z cache na disk zabere n�kolik minut).

Slo�ka bernoulli-mini obsahuje zjednodu�enou verzi knihovny,
psanou v C k�du. Knihovnu lze p�elo�it jako program MS VC++
2005 nebo za�lenit do projektu Raspberry Pico s ARM-GCC.
Knihovna vygeneruje prvn�ch 512 Bernoulliho ��sel (B2 a� B1024)
v C k�du (bernoulli.c a bernoulli.csv) a neumo��uje cachov�n�
v�po�tu.


Pozn�mky
--------
Dva identicky generovan� soubory Bernoulli.bin se mohou
li�it na offsetu 5 - polo�ka "loop", kter� nemus� b�t
spr�vn� obnovena p�i p�eru�en� a pokra�ov�n� generov�n�
(pro spr�vnou funk�nost nen� d�le�it�).

Program je vytvo�en v Microsoft Visual Studio VC++ 2005.
Program je kompilov�n v 64-bitov�m m�du x64. Je mo�n�
kompilace i pro 32-bitov� m�d, je v�ak zna�n� pomalej��
a m��e trp�t nedostatkem pam�ti.

Knihovna obsahuje ��sti k�du optimalizovan� assemblerem
x64, kter� jsou p�ekl�d�ny programem NASM 2.13.

Knihovna vznikla k v�po�tu Bernoulliho ��sel pro ��ely
knihovny pro v�po�et ��sel s plovouc� desetinnou ��rkou
s vysokou p�esnost�, konkr�tn� pro v�po�et velk�ch
line�rn�ch faktori�l�. Prvn�ch 512 Bernoulliho ��sel
(B2 a� B1024) umo��uje v�po�et line�rn�ch faktori�l�
s p�esnost� ��sel 4096 bit�, tj. 1224 ��slic. Pln�
rozsah 5000 Bernoulliho ��sel (B2 a� B10000) by bylo
mo�n� pou��t pro v�po�ty velk�ch line�rn�ch faktori�l�
s p�esnost� 40000 bit�, tj. 12240 ��slic.

Nejvy��� ��slo B10000 m� �itatel dlouh� 27691 ��slic
(jmenovatel je 13 ��slic) a zab�r� 11504 bajt�.
