cmake_minimum_required(VERSION 2.6)

project(X3D)

add_definitions(-D__x3d_build__)

if(${X3D_TARGET} STREQUAL "68k")
  INCLUDE(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(tigcc-x3d tigcc-x3d)
  
  add_definitions(-D__68k__)

  
  set(CMAKE_C_FLAGS "-std=c99")
  
  # Disable ranlib
  set(CMAKE_RANLIB "echo")

  # Force absolute paths
  set(CMAKE_USE_RELATIVE_PATHS FALSE)

  # Needed to make force the 'compiler' to tigcc-x3d
  include(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

  # Custom configuration for the archiver
  set(CMAKE_AR $ENV{X3D}/tools/tigcc-x3d)
  set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -ar -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -ar")

  # Custom configuration for the linker
  set(CMAKE_LINKER $ENV{X3D}/tools/tigcc-x3d)
  set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -n X3D")
  set(CMAKE_EXECUTABLE_SUFFIX "")

  # Lets the X3D source know to include the TIGCC header files
  add_definitions(-D__TIGCC_HEADERS__)

  if(${X3D_SUBTARGET} STREQUAL "ti92plus")
    set(CMAKE_C_FLAGS "-DUSE_TI92PLUS -O3 -Wall")
  elseif(${X3D_SUBTARGET} STREQUAL "v200")
    set(CMAKE_C_FLAGS "-DUSE_V200 -O3 -Wall")
  elseif(${X3D_SUBTARGET} STREQUAL "ti89")
    set(CMAKE_C_FLAGS "-DUSE_TI89 -O3 -Wall")
  endif()
elseif(${X3D_TARGET} STREQUAL "pc")
  set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  set(CMAKE_STATIC_LINKER_FLAGS, "-lm")
endif()




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
  C/memory/slaballocator.c
  C/memory/alloc.c
  C/memory/handle.c
  
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
  C/portal.c
  C/clipregion.c
)

add_subdirectory(C/platform)

add_library(X3D ${SRCS})
