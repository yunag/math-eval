cmake_minimum_required(VERSION 3.5)
project(math-eval)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerWarnings)

option(MATH_EVAL_BUILD_TESTS "Build tests" OFF)
option(MATH_EVAL_BUILD_EXAMPLES "Build examples" OFF)
option(MATH_EVAL_NOLOG "Disable logging" OFF)

add_subdirectory(deps)

add_library(parser
  src/symbol_table.c
  src/log.c
  src/parser.c
  src/token.c
  src/tokenizer.c
  src/evaluator.c
)
target_set_warnings(parser)

target_include_directories(parser
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(parser
  PRIVATE
  datastructs
)

if(MATH_EVAL_NOLOG)
  target_compile_definitions(parser PRIVATE MATH_EVAL_NOLOG)
endif()

if (MATH_EVAL_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if (MATH_EVAL_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
