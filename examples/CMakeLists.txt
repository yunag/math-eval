add_executable(app
  example.c
)

target_link_libraries(app 
  PRIVATE
  parser
)

add_executable(bench
  bench.c
)

target_link_libraries(bench 
  PRIVATE
  parser
)

target_compile_options(bench PRIVATE -O2)
