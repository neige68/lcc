# <CMakeLists.txt> -*- coding: utf-8 -*-
#
# Project lcc
# Copyright (C) 2024 neige68
#

cmake_minimum_required(VERSION 3.24)
set(CMAKE_VERBOSE_MAKEFILE ON)
# スタティックリンク
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(lcc CXX)

#------------------------------------------------------------
#
# boost
#

# BOOST を探す
# C: ドライブ以外から探すには 環境変数 CMAKE_PREFIX_PATH の設定が必要
find_package(Boost 1.84 REQUIRED COMPONENTS program_options)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#------------------------------------------------------------

# C++17 使用
set(CMAKE_CXX_STANDARD "17")

# /MP: マルチプロセス
# /W4: 警告レベルを4とする
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4")

# マップファイル生成
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MAP")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MAP")

# ローカルインクルードファイルの場所を追加
include_directories($ENV{LOCALINCLUDE})

add_compile_definitions(_CRT_RAND_S)

add_executable(lcc lcc.cpp)

# end of <CMakeLists.txt>
