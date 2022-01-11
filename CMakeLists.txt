cmake_minimum_required(VERSION 3.2)

project(evaluator)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(app)
