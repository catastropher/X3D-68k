cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{X3D}/bin)

add_subdirectory(tools)
add_subdirectory(src/static)
add_subdirectory(src/shared)