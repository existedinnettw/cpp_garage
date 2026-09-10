cmake_minimum_required(VERSION 3.8)
project(str_printer)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
# Nothing Conan-specific here. `conanrosenv.sh` puts the Conan CMakeDeps
# output on CMAKE_PREFIX_PATH and sets CMAKE_TOOLCHAIN_FILE, so this plain
# find_package resolves to the Conan-provided fmt (not the system one).
find_package(fmt REQUIRED)

add_library(str_printer src/str_printer.cpp)
target_include_directories(str_printer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(str_printer PUBLIC cxx_std_17)
target_link_libraries(str_printer PUBLIC fmt::fmt)

ament_export_targets(str_printerTargets HAS_LIBRARY_TARGET)
# Downstream ament packages get `find_dependency(fmt)` injected automatically.
ament_export_dependencies(fmt)

install(DIRECTORY include/ DESTINATION include)
install(
  TARGETS str_printer
  EXPORT str_printerTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

ament_package()
