project(fnnTests CXX)
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


cmake_path(GET fnnTests_SOURCE_DIR PARENT_PATH srcDir)
include_directories(${srcDir})

add_library(ari INTERFACE ${srcDir}/arithmetic.h)
target_compile_definitions(ari INTERFACE debug_arithmetic)
add_executable(ariTests test_ari.cpp)
target_compile_options(ariTests PRIVATE
        -frounding-math
        -Wall -Wextra
        ${flags}
)
target_link_libraries(ariTests PRIVATE ari)
target_link_libraries(ariTests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(ariTests PRIVATE fixed)

include(CTest)
include(Catch)
catch_discover_tests(ariTests)
