project(fnnTests CXX)
cmake_path(GET FixedPointNumbers PARENT_PATH srcDir)
include_directories(srcDir)


add_library(ari INTERFACE arithmetic.h)
target_compile_definitions(ari debug_arithmetic)

add_executable(ariTests test_ari.cpp)
target_link_libraries(ariTests PRIVATE ari)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(flags -fsanitize=undefined,shift,signed-integer-overflow -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(flags -Wno-parentheses)
else()
    set(flags "")
endif()
target_compile_options(ariTests PRIVATE
        -frounding-math
        -Wall -Wextra
        ${flags}
)

