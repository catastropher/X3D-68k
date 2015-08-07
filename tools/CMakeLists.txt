set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/tools)

project(tigcc-x3d)

add_executable(tigcc-x3d tigcc-x3d.cpp)

set_target_properties(tigcc-x3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/tools)