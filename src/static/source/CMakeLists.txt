cmake_minimum_required(VERSION 2.6)


INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

SET(CMAKE_AR $ENV{X3D}/tools/tigcc-x3d CACHE FILEPATH "Archiver")

SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -ar")

add_definitions(-D__TIGCC_HEADERS__)

Project(X3D)

include_directories($ENV{X3D}/src/static/headers)

add_library(X3D
  clip.c
  editor.c
  error.c
  math/fastsqrt.c
  math/fix.c
  frustum.c
  geo.c
  keys.c
  log.c
  matrix.c
  polygon.c
  prism.c
  render.c
  segment.c
  test.c
  math/trig.c
  math/vector.c
)
  