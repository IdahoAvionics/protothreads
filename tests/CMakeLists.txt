add_library(unity STATIC unity/unity.c)
target_include_directories(unity PUBLIC unity)

# Test executables using default lc-switch implementation
add_executable(test_pt_lifecycle test_pt_lifecycle.c)
target_link_libraries(test_pt_lifecycle PRIVATE protothreads unity)

add_executable(test_pt_waiting test_pt_waiting.c)
target_link_libraries(test_pt_waiting PRIVATE protothreads unity)

add_executable(test_pt_scheduling test_pt_scheduling.c)
target_link_libraries(test_pt_scheduling PRIVATE protothreads unity)

add_executable(test_pt_semaphore test_pt_semaphore.c)
target_link_libraries(test_pt_semaphore PRIVATE protothreads unity)

# Test lc-switch explicitly
add_executable(test_lc_switch test_lc_switch.c)
target_link_libraries(test_lc_switch PRIVATE protothreads unity)

# Test lc-addrlabels (GCC only)
add_executable(test_lc_addrlabels test_lc_addrlabels.c)
target_link_libraries(test_lc_addrlabels PRIVATE protothreads unity)
target_compile_definitions(test_lc_addrlabels PRIVATE LC_INCLUDE="lc-addrlabels.h")

# Register tests with CTest
add_test(NAME pt_lifecycle COMMAND test_pt_lifecycle)
add_test(NAME pt_waiting COMMAND test_pt_waiting)
add_test(NAME pt_scheduling COMMAND test_pt_scheduling)
add_test(NAME pt_semaphore COMMAND test_pt_semaphore)
add_test(NAME lc_switch COMMAND test_lc_switch)
add_test(NAME lc_addrlabels COMMAND test_lc_addrlabels)
