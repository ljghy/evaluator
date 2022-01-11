add_executable(eval)

target_sources(eval
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(eval
PRIVATE
    evaluator
)