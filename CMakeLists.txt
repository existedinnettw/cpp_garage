cmake_minimum_required(VERSION 3.15)
project(sobjectizer_test CXX)

find_package(sobjectizer CONFIG REQUIRED)

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

install(
  TARGETS sobjectizer_test
  DESTINATION "."
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
