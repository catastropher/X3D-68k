cmake_minimum_required(VERSION 2.6)

project(X3DShared)

# Needed to make force the 'compiler' to tigcc-x3d
INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

# Custom configuration for the linker
SET(CMAKE_LINKER $ENV{X3D}/tools/tigcc-x3d)

SET(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -n X3D")

SET(CMAKE_EXECUTABLE_SUFFIX "")

# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)

include_directories($ENV{X3D}/src/static/headers)

add_executable(X3DShared
	source/export.c
	source/init/init.c
)

target_link_libraries(X3DShared $ENV{X3D}/lib/libX3D.a)
target_link_libraries(X3DShared $ENV{X3D}/lib/extgraph.a)

set_target_properties(X3DShared PROPERTIES OUTPUT_NAME X3D.9xy)
set_target_properties(X3DShared PROPERTIES RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/lib)