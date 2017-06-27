#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/tools)

cmake_minimum_required(VERSION 2.6)

project(palette)
add_executable(palette palette.cpp)

project(singen)
add_executable(singen singen.cpp)

project(convert-tex)
add_executable(convert-tex convert-tex.c)
include_directories(/usr/local/include/X3D)
find_package(SDL REQUIRED)

target_link_libraries(convert-tex X3D SDL m)