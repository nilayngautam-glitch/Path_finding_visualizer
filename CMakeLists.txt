cmake_minimum_required(VERSION 3.16)
project(pathfinder CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(core INTERFACE)
target_include_directories(core INTERFACE src)

add_executable(tests tests/test_core.cpp)
target_link_libraries(tests core)