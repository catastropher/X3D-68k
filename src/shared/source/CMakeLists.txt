cmake_minimum_required(VERSION 2.6)


INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(tigcc-x3d tigcc-x3d)

add_definitions(-D__TIGCC_HEADERS__)

Project(X3D)

include_directories(${X3D_SOURCE_DIR}/../headers/)

add_library(X3D
  clip.c
  editor.c
  error.c
  fastsqrt.c
  fix.c
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
  trig.c
  vector.c
)
  