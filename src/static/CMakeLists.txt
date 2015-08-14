cmake_minimum_required(VERSION 2.6)

Project(X3DStatic)

add_definitions(-D__TIGCC_STATIC__)

add_library(X3DStatic
  source/init/init.c
  source/debug/error.c
)

if(X3D_TARGET EQUAL 68k)
  include($ENV{X3D}/src/platform/68k/68k.cmake)
  
elseif(X3D_TARGET EQUAL "pc")
endif()



set_target_properties(X3DStatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})




include_directories($ENV{X3D}/src/static/headers)
include_directories($ENV{X3D}/src/shared/headers)
include_directories($ENV{X3D}/lib)
include_directories(${PLATFORM_DIR})

set(CMAKE_USE_RELATIVE_PATHS FALSE)



set_target_properties(X3DStatic PROPERTIES OUTPUT_NAME X3D)
