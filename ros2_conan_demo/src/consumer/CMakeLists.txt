cmake_minimum_required(VERSION 3.8)
project(consumer)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)  # from this package's own conanfile.py
# str_printer's exported config pulls in fmt via find_dependency(); no direct
# mention of fmt or Conan is needed in this package at all.
find_package(str_printer REQUIRED)

add_executable(main src/main.cpp)
target_compile_features(main PUBLIC cxx_std_17)
target_link_libraries(main PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  nlohmann_json::nlohmann_json
  str_printer::str_printer)

install(TARGETS main DESTINATION lib/${PROJECT_NAME})

ament_package()
