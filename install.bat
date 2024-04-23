@rem install.bat -*- coding: cp932-dos -*-
@echo off
rem
rem Project lcc
rem Copyright (C) 2024 neige68
rem
rem Note: install
rem
rem Windows:   10 and later
rem

setlocal
title %~f0
pushd %~dp0

replace /a build\release\lcc.exe %LOCALBIN%
replace /u build\release\lcc.exe %LOCALBIN%

for %%i in (%LOCALBIN%\lcc.exe) do set @SRC=%%~ti
for %%i in (%LOCALBIN%\ltu.exe) do set @DST=%%~ti
if "%@DST%" GEQ "%@SRC%" goto skip_gen_dst
copy %LOCALBIN%\lcc.exe %LOCALBIN%\lccu.exe
:skip_gen_dst

@rem end of install.bat
