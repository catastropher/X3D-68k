cmake_minimum_required(VERSION 2.6)

project(X3D)

add_definitions(-D__x3d_build__)

if(${X3D_TARGET} STREQUAL "68k")
  INCLUDE(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(tigcc-x3d tigcc-x3d)
  
  add_definitions(-D__68k__)
  include_directories($ENV{X3D}/lib)
  
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
  add_definitions(-D__pc__)
  
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0 AND NOT APPLE)
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  else()
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -Wno-pointer-sign")
  endif()

  set(CMAKE_STATIC_LINKER_FLAGS, "-lm")
  
elseif(${X3D_TARGET} STREQUAL "nspire")
  set(CMAKE_C_COMPILER nspire-gcc)
  add_definitions(-D__nspire__)
  set(CMAKE_C_FLAGS "-Wall -O3")
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
  C/engine/gameloop.c
  C/engine/log.c
  C/engine/init.c
  C/engine/enginestate.c
  
  C/memory/freeslist.c
  C/memory/stack.c
  C/memory/list.c
  C/memory/varsizeallocator.c
  C/memory/slaballocator.c
  C/memory/alloc.c
  C/memory/handle.c
  
  C/math/vector.c
  C/math/trig.c
  C/math/matrix.c
  C/math/fastsqrt.c
  
  C/geo/prism.c
  C/geo/plane.c
  C/geo/polygon.c
  C/geo/line.c
  
  C/level/level.c
  C/level/level_vertex.c
  C/level/level_face.c
  C/level/level_segment.c
  
  C/render/render.c
  C/render/clip2.c
  C/render/wallportal.c
  C/render/portal.c
  C/render/clip.c
  C/render/rasteredge.c
  C/render/util.c
  C/render/scanline.c
  C/render/polyline.c
  C/render/texture.c
  C/render/font.c
  C/render/final_clip.c
  C/render/render_hud.c
  C/render/render_line.c
  
  C/physics/collide.c
  C/physics/raytrace.c
  
  C/object/object.c
  C/object/camera.c
  
  C/clipper/clipper.c
)

add_subdirectory(C/platform)

add_library(X3D ${SRCS})
