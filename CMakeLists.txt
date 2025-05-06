cmake_minimum_required(VERSION 3.13)

project(fast_cpp_sort)

set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(fast_cpp_sort main.cpp)
