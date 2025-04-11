cmake_minimum_required(VERSION 3.30)
project(FixedPointPrecision CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER_ID Clang)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
add_executable(test main.cpp
        fixed.cxx)
