cmake_minimum_required(VERSION 2.6)

Project(X3DUtil)

add_definitions(-D__TIGCC_STATIC__)

add_library(X3DUtil
  C/init/init.c
)

include(${PLATFORM_DIR}/cmake/platform.cmake)


set_target_properties(X3DUtil PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})


include_directories($ENV{X3D}/src/util/headers)
include_directories($ENV{X3D}/src/core/headers)
include_directories($ENV{X3D}/lib)
include_directories(${PLATFORM_DIR}/headers)

set(CMAKE_USE_RELATIVE_PATHS FALSE)



set_target_properties(X3DUtil PROPERTIES OUTPUT_NAME X3DUtil)
