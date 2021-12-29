cmake_minimum_required(VERSION 3.2)
project(evaluator)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
include_directories(./include)

add_executable(eval src/Context.cpp 
                    src/Function.cpp
                    src/Tokenizer.cpp
                    src/main.cpp)

add_library(evaluator STATIC src/Context.cpp 
                            src/Function.cpp
                            src/Tokenizer.cpp)
