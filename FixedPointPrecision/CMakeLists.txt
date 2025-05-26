cmake_minimum_required(VERSION 3.30)
project(FixedPointPrecision CXX)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_COMPILER_ID GNU)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(TARGET_H)
add_library(fixed STATIC
        arithmetic.h)
target_sources(fixed
    PUBLIC
        FILE_SET CXX_MODULES FILES
            fixed.cxx)
target_compile_options(fixed PUBLIC -fsanitize=undefined -fsanitize=signed-integer-overflow)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE fixed)
