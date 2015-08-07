cmake_minimum_required(VERSION 2.6)

Project(X3DStatic)

# Needed to make force the 'compiler' to tigcc-x3d
INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

# Custom configuration for the archiver
SET(CMAKE_AR $ENV{X3D}/tools/tigcc-x3d)
SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -ar -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -ar")

# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)

include_directories($ENV{X3D}/src/static/headers)

add_library(X3DStatic
  source/clip.c
  source/editor.c
  source/error.c
  source/math/fastsqrt.c
  source/math/fix.c
  source/frustum.c
  source/geo.c
  source/keys.c
  source/log.c
  source/matrix.c
  source/polygon.c
  source/prism.c
  source/render.c
  source/segment.c
  source/test.c
  source/math/trig.c
  source/math/vector.c
  source/init/init.c
)


set_target_properties(X3DStatic PROPERTIES OUTPUT_NAME X3D)
set_target_properties(X3DStatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY $ENV{X3D}/lib)