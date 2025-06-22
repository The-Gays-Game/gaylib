cmake_minimum_required(VERSION 3.30)
project(FixedPointNumbers CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
add_compile_definitions(debug_arithmetic)
add_subdirectory(tests)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(flags -fsanitize=undefined,shift,signed-integer-overflow -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(flags -Wno-parentheses)
else()
	set(flags "")
endif()

add_library(fixed STATIC)
target_sources(fixed
    PUBLIC
        FILE_SET CXX_MODULES FILES
            fixed.cxx)
target_link_libraries(fixed PUBLIC ari)


target_compile_options(
		fixed PUBLIC
		-Wall -Wextra #-fsanitize=undefined -save-temps -v
		${flags}
)
add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE fixed ari)

