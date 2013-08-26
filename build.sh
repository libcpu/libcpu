#!/bin/sh

rm -f	CMakeCache.txt
rm -rf	CMakeFiles
rm -f	cmake_install.cmake

cmake -D CMAKE_CXX_COMPILER=`which clang++` -D CMAKE_C_COMPILER=`which clang` CMakeLists.txt
