# Zephyr application. Nothing here is Conan-specific: Zephyr owns the toolchain,
# Kconfig and devicetree. Conan only makes find_package(spdlog) / find_package(fmt)
# succeed by putting its generated <pkg>-config.cmake files on CMAKE_PREFIX_PATH.
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_conan LANGUAGES C CXX)

target_sources(app PRIVATE src/main.cpp)

# spdlog where the C++ standard library has threads (native_sim), otherwise
# spdlog's formatting core, fmt (see conanfile.py for the reason).
find_package(spdlog QUIET)
if(spdlog_FOUND)
  target_link_libraries(app PRIVATE spdlog::spdlog)
  target_compile_definitions(app PRIVATE APP_HAVE_SPDLOG=1)
else()
  find_package(fmt REQUIRED)
  target_link_libraries(app PRIVATE fmt::fmt)
endif()
