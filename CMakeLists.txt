cmake_minimum_required(VERSION 2.6)

set(CMAKE_USE_RELATIVE_PATHS FALSE)

add_subdirectory(tools)
add_subdirectory(src/static)
add_subdirectory(src/shared)
add_subdirectory(test)