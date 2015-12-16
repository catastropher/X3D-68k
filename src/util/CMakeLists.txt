cmake_minimum_required(VERSION 2.6)

Project(X3DUtil)

add_definitions(-D__TIGCC_STATIC__)

add_library(X3DUtil
  C/init/init.c
  C/interface.c
)

if(X3D_TARGET EQUAL 68k)
  include($ENV{X3D}/src/platform/68k/68k.cmake)
  
elseif(X3D_TARGET EQUAL "pc")
endif()



set_target_properties(X3DUtil PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})




include_directories($ENV{X3D}/src/util/headers)
include_directories($ENV{X3D}/src/core/headers)
include_directories($ENV{X3D}/lib)
include_directories(${PLATFORM_DIR})

set(CMAKE_USE_RELATIVE_PATHS FALSE)



set_target_properties(X3DUtil PROPERTIES OUTPUT_NAME X3DUtil)
