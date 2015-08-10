cmake_minimum_required(VERSION 2.6)

Project(X3DStatic)

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

if(X3D_TARGET EQUAL 68k)
  include($ENV{X3D}/src/platform/68k/68k.cmake)
  
elseif(X3D_TARGET EQUAL "pc")
endif()



set_target_properties(X3DStatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})




include_directories($ENV{X3D}/src/static/headers)
include_directories($ENV{X3D}/lib)

set(CMAKE_USE_RELATIVE_PATHS FALSE)



set_target_properties(X3DStatic PROPERTIES OUTPUT_NAME X3D)
