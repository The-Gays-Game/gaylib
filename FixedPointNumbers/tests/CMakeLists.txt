project(fpnTests CXX)
Include(FetchContent)

option(CATCH_CONFIG_WINDOWS_CRTDBG ON)
option(CATCH_BUILD_TESTING OFF)
option(CATCH_CONFIG_WINDOWS_SEH ON)
FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        88abf9bf325c798c33f54f6b9220ef885b267f4f#v3.12
)
FetchContent_MakeAvailable(Catch2)
#configure_file(${catch2_SOURCE_DIR}/extras/gdbinit ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit)


cmake_path(GET fpnTests_SOURCE_DIR PARENT_PATH srcDir)
include_directories(${srcDir})
set(dbgOpts
		-frounding-math -ffp-contract=on -fstrict-enums -fstrict-aliasing -fshort-enums #-Og
		-Wall -Wextra)
add_library(testDbg STATIC)
target_sources(testDbg PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS ${srcDir}
		FILES ../core.ixx ../types.ixx ../arithmetic.cxx

		#PRIVATE
)
target_compile_options(testDbg PUBLIC
		${dbgOpts}
		${flags}
)
target_compile_definitions(testDbg PRIVATE checkArgs dbgNo128)


add_library(helpers STATIC)
target_sources(helpers PUBLIC
		FILE_SET CXX_MODULES
		FILES helpers.ixx
)
target_compile_options(helpers PUBLIC
		${dbgOpts}
		${flags}
)

add_executable(fpnRTTests core_rt.cpp)
target_link_libraries(fpnRTTests PRIVATE testDbg)
target_link_libraries(fpnRTTests PRIVATE helpers)
target_link_libraries(fpnRTTests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(fpnRTTests)
