#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/tools)

cmake_minimum_required(VERSION 2.6)

project(tigcc-x3d)

add_executable(tigcc-x3d tigcc-x3d.cpp)

set_target_properties(tigcc-x3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/tools)

project(palette)
add_executable(palette palette.cpp)

project(model-convert)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
add_executable(model-convert model-convert.cpp)

project(singen)
add_executable(singen singen.cpp)