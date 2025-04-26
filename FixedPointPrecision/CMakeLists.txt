cmake_minimum_required(VERSION 3.30)
project(FixedPointPrecision CXX)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_COMPILER_ID GNU)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
add_library(fixed)
target_sources(fixed
    PUBLIC
        FILE_SET CXX_MODULES FILES
            fixed.cxx)
#set_source_files_properties(fixed.cxx PROPERTIES COMPILE_OPTIONS -freport-bug)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE fixed)
