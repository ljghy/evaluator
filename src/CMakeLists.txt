add_library(evaluator STATIC)

target_sources(evaluator
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Function.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tokenizer.cpp
)

target_include_directories(evaluator
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)