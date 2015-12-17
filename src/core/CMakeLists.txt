cmake_minimum_required(VERSION 2.6)

project(X3DCore)

set(X3D_CORE_SOURCES C/init/export.c
  C/debug/error.c
  C/init/init.c
  C/timer/timer.c
  C/memory/freelist.c
  C/interface.c)



# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)
add_definitions(-D__X3D_SHARED__)

include_directories(${PLATFORM_DIR}/headers)
include_directories($ENV{X3D}/src/core/headers)
include_directories($ENV{X3D}/lib)

if(X3D_TARGET EQUAL 68k)
  add_executable(X3DCore
    ${X3D_CORE_SOURCES}
  )
  
  set_target_properties(X3DCore PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
  set_target_properties(X3DCore PROPERTIES OUTPUT_NAME X3D.9xy)
else()
  add_library(X3DCore
    ${X3D_CORE_SOURCES}
  )
  
  set_target_properties(X3DCore PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endif()

include(${PLATFORM_DIR}/cmake/platform.cmake)


target_link_libraries(X3DCore $ENV{X3D}/lib/extgraph.a)
target_link_libraries(X3DCore $ENV{X3D}/lib/68k/ti92plus/libX3DPlatform.a)



