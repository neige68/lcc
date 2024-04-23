@rem build.bat -*- coding: cp932-dos -*-
@echo off
rem
rem Project lcc
rem Copyright (C) 2024 neige68
rem
rem Note: build
rem
rem Windows: 10 and later
rem

setlocal
pushd %~dp0

rem VC の vcvarsall.bat のあるディレクトリを指定
set @VC=%VC142%
set @CMAKEOPT=-G "Visual Studio 16 2019" -A Win32 

set @exec_cmake=
set @bdir=build
set @build_deb=t
set @build_rel=t

:opt_loop
if "%1"=="" goto opt_end
if "%1"=="cm" set @exec_cmake=t
if "%1"=="cm" goto opt_next
if "%1"=="rebuild" goto opt_rebuild
if "%1"=="reb" goto opt_rebuild
if "%1"=="re" goto opt_rebuild
if "%1"=="clean" goto opt_rebuild
if "%1"=="d" set @build_rel=
if "%1"=="d" goto opt_next
if "%1"=="deb" set @build_rel=
if "%1"=="deb" goto opt_next
if "%1"=="r" set @build_deb=
if "%1"=="r" goto opt_next
if "%1"=="rel" set @build_deb=
if "%1"=="rel" goto opt_next
echo WARN: build.bat: %1 を無視します.
:opt_next
shift
goto opt_loop
:opt_rebuild
if not exist %@bdir% echo INFO: build.bat: %@bdir% はありません.
if exist %@bdir% echo INFO: build.bat: %@bdir% を削除しています...
if exist %@bdir% rmdir /s /q %@bdir%
if exist %@bdir% echo ERROR: build.bat: %@bdir% の削除ができませんでした.
if exist %@bdir% goto end
set @exec_cmake=t
shift
goto opt_loop
:opt_end

if not "%VCToolsVersion%"=="" echo INFO: build.bat: VCToolsVersion が設定されています. このまま実行します.
if "%VCToolsVersion%"=="" call "%@VC%\vcvarsall.bat" x86
if not exist %@bdir% mkdir %@bdir%
pushd %@bdir%
if not exist ALL_BUILD.vcxproj set @exec_cmake=t
if not "%@exec_cmake%"=="" cmake %@CMAKEOPT% ..
if not "%@build_deb%"=="" msbuild ALL_BUILD.vcxproj /p:Configuration=Debug /m
if errorlevel 1 goto err
echo INFO: build.bat: msbuild Debug Done.
if not "%@build_rel%"=="" msbuild ALL_BUILD.vcxproj /p:Configuration=Release /m
if errorlevel 1 goto err
echo INFO: build.bat: msbuild Release Done.
popd
goto end
:err
echo ERROR: build.bat: エラーがあります.
popd

:end
popd
if errorlevel 1 exit /b 1

rem end of build.bat
