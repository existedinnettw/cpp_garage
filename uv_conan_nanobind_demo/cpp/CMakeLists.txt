# Plain C++ library, usable on its own (no Python involved).
find_package(fmt REQUIRED CONFIG)

add_library(geom STATIC src/geom.cpp)
target_include_directories(geom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(geom PUBLIC fmt::fmt)
