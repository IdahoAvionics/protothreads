add_executable(example-small example-small.c)
target_link_libraries(example-small PRIVATE protothreads)

add_executable(example-buffer example-buffer.c)
target_link_libraries(example-buffer PRIVATE protothreads)

add_executable(example-codelock example-codelock.c)
target_link_libraries(example-codelock PRIVATE protothreads)
