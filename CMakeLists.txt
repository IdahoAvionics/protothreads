cmake_minimum_required(VERSION 3.10)
project(protothreads VERSION 1.4 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(protothreads INTERFACE)
target_include_directories(protothreads INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Only build examples and tests when this is the top-level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(BUILD_EXAMPLES "Build example programs" ON)
    option(BUILD_TESTING "Build tests" ON)

    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
