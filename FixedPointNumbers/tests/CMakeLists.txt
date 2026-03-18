project(fpnTests CXX)

find_package(Catch2 3 REQUIRED)
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
target_link_libraries(helpers PRIVATE testDbg)

add_executable(fpnRTTests core_rt.cpp)
target_link_libraries(fpnRTTests PRIVATE testDbg)
target_link_libraries(fpnRTTests PRIVATE helpers)
target_link_libraries(fpnRTTests PRIVATE Catch2::Catch2WithMain)
target_compile_options(fpnRTTests PRIVATE -Og)
include(CTest)
include(Catch)
catch_discover_tests(fpnRTTests)
