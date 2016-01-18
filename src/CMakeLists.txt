cmake_minimum_required(VERSION 2.6)

project(X3D)

set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined")

macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

include_directories(headers)
include_directories(C/src/platform)

add_sources(
  C/gameloop.c
  C/log.c
  C/init.c
  C/resource.c
  C/vector.c
  
  C/memory/freeslist.c
  C/memory/stack.c
  C/memory/list.c
  C/memory/varsizeallocator.c
  
  C/enginestate.c
  C/segment.c
  C/segmentbuilder.c
  C/trig.c
  C/matrix.c
  C/prism.c
  C/render.c
  
  C/plane.c
  C/fastsqrt.c
  C/polygon.c
  C/clip.c
  C/collide.c
  C/object.c
  C/camera.c
  C/wallportal.c
)

add_subdirectory(C/platform)

add_library(X3D ${SRCS})