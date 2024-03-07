cmake_minimum_required(VERSION 3.15)
project(sobjectizer_test CXX)

find_package(sobjectizer CONFIG REQUIRED)
find_package(benchmark REQUIRED)

add_executable(sobjectizer_test src/main.cpp)
add_executable(csp_ping_pong src/csp_ping_pong.cpp)

# target_link_libraries(sobjectizer_test PRIVATE sobjectizer::sobjectizer)

if(TARGET sobjectizer::SharedLib)
  target_link_libraries(sobjectizer_test sobjectizer::SharedLib)
  target_link_libraries(csp_ping_pong sobjectizer::SharedLib)
else()
  target_link_libraries(sobjectizer_test sobjectizer::StaticLib)
  target_link_libraries(csp_ping_pong sobjectizer::StaticLib)
endif()

target_link_libraries(csp_ping_pong benchmark::benchmark)

# --
find_package(Threads REQUIRED)
add_executable(copper_ping_pong src/copper_ping_pong.cpp)
include(FetchContent)
FetchContent_Declare(
  copper
  GIT_REPOSITORY https://github.com/atollk/copper
  GIT_TAG v1.1)
FetchContent_MakeAvailable(copper)
target_link_libraries(copper_ping_pong Threads::Threads copper
                      benchmark::benchmark)

install(
  TARGETS sobjectizer_test
  DESTINATION "."
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
