cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/lib)
set(CALC_NAME test)

project(test)

# Needed to make force the 'compiler' to tigcc-x3d
INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

# Custom configuration for the linker
SET(CMAKE_LINKER $ENV{X3D}/tools/tigcc-x3d)
SET(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -n test")
SET(CMAKE_EXECUTABLE_SUFFIX "")

# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)

include_directories($ENV{X3D}/lib/x3d/headers)
include_directories($ENV{X3D}/lib)
include_directories(${PLATFORM_DIR}/headers)

add_executable(X3D-test
	test.c
)

set_target_properties(X3D-test PROPERTIES OUTPUT_NAME test.9xz)

target_link_libraries(X3D-test $ENV{X3D}/lib/68k/ti92plus/libX3DUtil.a)
target_link_libraries(X3D-test $ENV{X3D}/lib/extgraph.a)