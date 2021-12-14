cmake_minimum_required(VERSION 3.2)
project(evaluator)
set(CMAKE_CXX_FLAGS "-std=c++17 -O2")
add_compile_options(-Wall)
include_directories(./include)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
add_executable(eval ${SRC_LIST})
