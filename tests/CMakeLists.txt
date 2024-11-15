enable_testing()

add_executable(test
  test.c
)

find_package(Python REQUIRED COMPONENTS Interpreter)

# NOTE: The goal of this test is compare python's `eval` with `math-eval`
add_test (NAME python-eval-test
  COMMAND ${Python_EXECUTABLE} test.py "$<TARGET_FILE:test>"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test
  PRIVATE
  datastructs
  parser
)
