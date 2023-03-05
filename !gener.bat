@echo off
set BERN_NUM=5000
Bernoulli_x64.exe %BERN_NUM% 1 bern_tab.csv
Bernoulli_x64.exe %BERN_NUM% 2 bern_com.csv
Bernoulli_x64.exe %BERN_NUM% 3 bern_sem.csv
Bernoulli_x64.exe %BERN_NUM% 4 bern_const.cpp
