cmake_minimum_required(VERSION 3.30)
project(FixedPointNumbers CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
add_library(fixed STATIC
        #arithmetic.h
)
target_sources(fixed
    PUBLIC
        FILE_SET CXX_MODULES FILES
            fixed.cxx)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(flags -fsanitize=undefined,shift,signed-integer-overflow -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(flags -Wno-parentheses)
else()
	set(flags "")
endif()
target_compile_options(
		fixed PUBLIC
		-Wall -Wextra #-fsanitize=undefined -save-temps -v
		${flags}
)
add_executable(demo main.cpp)
target_link_libraries(demo PUBLIC fixed)

Include(FetchContent)
FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        2b60af89e23d28eefc081bc930831ee9d45ea58b#v3.8.1
)
set(CATCH_CONFIG_WINDOWS_CRTDBG ON)
set(CATCH_BUILD_TESTING OFF)
set(CATCH_CONFIG_WINDOWS_SEH ON)
FetchContent_MakeAvailable(Catch2)
#configure_file(${catch2_SOURCE_DIR}/extras/gdbinit ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit)
add_executable(tests test_ari.cpp)
target_compile_options(
		tests PUBLIC
		-frounding-math
		-Wall -Wextra
		${flags}
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE fixed)
include(CTest)
include(Catch)
catch_discover_tests(tests)