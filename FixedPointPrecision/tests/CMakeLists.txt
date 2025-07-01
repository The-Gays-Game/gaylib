project(fnnTests CXX)
Include(FetchContent)

option(CATCH_CONFIG_WINDOWS_CRTDBG ON)
option(CATCH_BUILD_TESTING OFF)
option(CATCH_CONFIG_WINDOWS_SEH ON)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        2b60af89e23d28eefc081bc930831ee9d45ea58b#v3.8.1
)
FetchContent_MakeAvailable(Catch2)
#configure_file(${catch2_SOURCE_DIR}/extras/gdbinit ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit)


cmake_path(GET fnnTests_SOURCE_DIR PARENT_PATH srcDir)
include_directories(${srcDir})

add_library(t_ari INTERFACE ${srcDir}/arithmetic.h)
target_compile_definitions(t_ari INTERFACE checkArgs)
target_compile_options(t_ari INTERFACE -fstrict-enums -fstrict-aliasing -fshort-enums)

add_library(t_fixed STATIC)
target_sources(t_fixed PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${srcDir}
        FILES ../fixed.cxx
)
target_compile_options(t_fixed PUBLIC
        -frounding-math -ffp-contract=on
        -Wall -Wextra
        ${flags}
)
target_compile_definitions(t_fixed PRIVATE t_fixed_cxx)
target_link_libraries(t_fixed PUBLIC t_ari)

add_library(helpers STATIC)
target_sources(helpers PUBLIC
        FILE_SET CXX_MODULES
        FILES helpers.cxx
)
target_compile_options(helpers PUBLIC
        -frounding-math -ffp-contract=on -fstrict-enums -fstrict-aliasing -fshort-enums
        -Wall -Wextra
        ${flags}
)

add_executable(ariTests test_ari.cpp)
target_link_libraries(ariTests PRIVATE t_ari)
target_link_libraries(ariTests PRIVATE helpers)
target_link_libraries(ariTests PRIVATE Catch2::Catch2WithMain)

add_executable(fpnTests test_fpn.cpp)
target_link_options(fpnTests
        PRIVATE -fuse-ld=lld#gcc's ld fails when fold expands to too many expression with catch2.
)
target_link_libraries(fpnTests PRIVATE t_fixed)
target_link_libraries(fpnTests PRIVATE t_ari)
target_link_libraries(fpnTests PRIVATE helpers)
target_link_libraries(fpnTests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(ariTests)
catch_discover_tests(fpnTests)
