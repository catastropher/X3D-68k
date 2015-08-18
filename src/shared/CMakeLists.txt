cmake_minimum_required(VERSION 2.6)

project(X3DShared)



# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)
add_definitions(-D__X3D_SHARED__)

include_directories(${PLATFORM_DIR})
include_directories($ENV{X3D}/src/shared/headers)

add_executable(X3DShared
	source/export.c
	source/init/init.c
  source/debug/log.c
  source/engine.c
  source/debug/error.c
  source/resource/memory.c
  source/trig.c
  source/objects.c
)

if(X3D_TARGET EQUAL 68k)
  include($ENV{X3D}/src/platform/68k/68k.cmake)
  
elseif(X3D_TARGET EQUAL "pc")
endif()


target_link_libraries(X3DShared $ENV{X3D}/lib/extgraph.a)

set_target_properties(X3DShared PROPERTIES OUTPUT_NAME X3D.9xy)
set_target_properties(X3DShared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})