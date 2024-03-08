cmake_minimum_required(VERSION 3.15)
project(sobjectizer_test CXX)

find_package(sobjectizer CONFIG REQUIRED)
find_package(benchmark REQUIRED)

add_executable(so5_actor src/so5_actor.cpp)
add_executable(so5_ping_pong src/so5_ping_pong.cpp)

# target_link_libraries(so5_actor PRIVATE sobjectizer::sobjectizer)

if(TARGET sobjectizer::SharedLib)
  target_link_libraries(so5_actor sobjectizer::SharedLib)
  target_link_libraries(so5_ping_pong sobjectizer::SharedLib)
else()
  target_link_libraries(so5_actor sobjectizer::StaticLib)
  target_link_libraries(so5_ping_pong sobjectizer::StaticLib)
endif()

target_link_libraries(so5_ping_pong benchmark::benchmark)

# ------------------------
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

# -------------------------
find_package(asiochan CONFIG REQUIRED)

add_executable(asiochan_ping_pong src/asiochan_ping_pong.cpp)
target_link_libraries(asiochan_ping_pong asiochan::asiochan benchmark::benchmark)

