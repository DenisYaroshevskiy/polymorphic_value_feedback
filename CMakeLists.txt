cmake_minimum_required(VERSION 3.7)

SET(CMAKE_CXX_COMPILER clang++-mp-7.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_subdirectory(third_party)

project(polymorfic_value)

include_directories(./ ./third_party)

add_compile_options(-Wall -Wextra -Werror -Wpedantic -g)

add_executable(benchmarks)
target_sources(benchmarks PRIVATE
               google_benchmark_main.cc)
set_target_properties(benchmarks PROPERTIES CXX_STANDARD 17)
target_compile_options(benchmarks PRIVATE -O3)
target_link_libraries(benchmarks benchmark)

add_executable(tests)
set_target_properties(benchmarks PROPERTIES CXX_STANDARD 14)
target_sources(tests PRIVATE
               jbcoe_polymorphic_value_tests_for_simple.cpp
               catch_main.cc)
