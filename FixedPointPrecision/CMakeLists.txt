cmake_minimum_required(VERSION 3.30)
project(FixedPointPrecision CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
add_compile_definitions(debug_arithmetic)
add_library(fixed STATIC
        arithmetic.h)
target_sources(fixed
    PUBLIC
        FILE_SET CXX_MODULES FILES
            fixed.cxx)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(flags -fsanitize=undefined -fsanitize=shift -fsanitize=signed-integer-overflow -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(flags -Wno-parentheses)
else()
	set(flags "")
endif()
target_compile_options(
		fixed PUBLIC
		-Wall -Wextra
		${flags}
)
add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE fixed)

#find_package(Catch2 3 REQUIRED)

#add_executable(tests test_ari.cpp)
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
#target_link_libraries(tests PRIVATE fixed)