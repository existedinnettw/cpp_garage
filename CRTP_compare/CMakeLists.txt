
cmake_minimum_required (VERSION 3.15)
project (CRTP_compare)

find_package(benchmark REQUIRED)

add_executable(bench main.cpp)
target_include_directories(bench PUBLIC .)

target_link_libraries(bench benchmark::benchmark)


