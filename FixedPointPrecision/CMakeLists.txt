cmake_minimum_required(VERSION 3.30)
project(FixedPointNumbers
		VERSION 0.0.1
		DESCRIPTION "A correct, modern, fast fixed point decimal library using C++20."
		LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
add_compile_definitions(checkArgs)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(flags -fsanitize=undefined,shift,signed-integer-overflow -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(flags -Wno-parentheses -fnon-call-exceptions -fipa-strict-aliasing -ftrapv -fconcepts-diagnostics-depth=2)
else()
	set(flags "")
endif()
add_subdirectory(tests)
add_library(fixed STATIC)
target_sources(fixed
		#PUBLIC
    	PUBLIC FILE_SET CXX_MODULES FILES fixed.cxx
		)
target_compile_definitions(fixed PRIVATE t_fixed_cxx)


target_compile_options(
		fixed PUBLIC
		-Wall -Wextra -ffp-contract=on -frounding-math -g
		${flags}
)
add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE fixed)

