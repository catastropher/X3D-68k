cmake_minimum_required(VERSION 2.6)

Project(X3DPlatform)

add_definitions(-D__TIGCC_STATIC__)

add_library(X3DPlatform
  C/init/init.c
  C/debug/error.c
  C/screen/screen.c
  C/debug/log.c
  C/keys.c
)

include(${PLATFORM_DIR}/cmake/platform.cmake)


set_target_properties(X3DPlatform PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

include_directories(${INTERFACE_DIR})
include_directories(${PLATFORM_DIR}/headers)
include_directories($ENV{X3D}/src/platform/pc/headers)
include_directories($ENV{X3D}/src/headers)
include_directories($ENV{X3D}/src/headers/platform)
include_directories($ENV{X3D}/lib)

set(CMAKE_USE_RELATIVE_PATHS FALSE)

set_target_properties(X3DPlatform PROPERTIES OUTPUT_NAME X3DPlatform)