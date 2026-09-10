cmake_minimum_required(VERSION 3.15)
project(uv_conan_nanobind_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # static lib gets linked into a .so

# --- 1. The underlying C++ library (depends on fmt from Conan) -------------
add_subdirectory(cpp)

# --- 2. The nanobind extension that wraps it ------------------------------
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_core bindings/module.cpp)
target_link_libraries(_core PRIVATE geom)

# Destination must match the Python package name so the .so lands next to
# src/uv_conan_nanobind_demo/__init__.py inside the wheel.
install(TARGETS _core DESTINATION uv_conan_nanobind_demo)
