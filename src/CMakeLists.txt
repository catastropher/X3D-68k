cmake_minimum_required(VERSION 2.6)

project(X3D C ASM)

if(NOT DEFINED X3D_TARGET)
    set(X3D_TARGET "pc")
endif()

if(${X3D_TARGET} STREQUAL "pc")
  add_definitions(-D__pc__)
  
  message(STATUS "Building X3D for PC")
  
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0 AND NOT APPLE)
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  else()
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -Wno-pointer-sign")
  endif()

  set(CMAKE_STATIC_LINKER_FLAGS, "-lm")  
elseif(${X3D_TARGET} STREQUAL "nspire")
  message(STATUS "Building X3D for nspire")

  set(CMAKE_C_COMPILER nspire-gcc)
  add_definitions(-D__nspire__)
  set(CMAKE_C_FLAGS "-Wall -O3")
  set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")
  set(CMAKE_ASM_COMPILER nspire-as)
endif()


macro(add_sources)
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
  
  C/file/buffer.c
  C/file/resourcepack.c
  
  C/math/vector.c
  C/math/trig.c
  C/math/matrix.c
  C/math/fastsqrt.c
  
  C/geo/prism.c
  C/geo/plane.c
  C/geo/polygon.c
  C/geo/line.c
  C/geo/model.c
  
  C/level/level.c
  C/level/level_vertex.c
  C/level/level_face.c
  C/level/level_segment.c
  C/level/level_linetexture.c
  C/level/levelbuilder.c
  
  C/render/render.c
  C/render/wallportal.c
  C/render/portal.c
  C/render/clip.c
  C/render/util.c
  C/render/polyline.c
  C/render/texture.c
  C/render/font.c
  C/render/final_clip.c
  C/render/render_hud.c
  C/render/lightmap.c
  
  C/render/geo/render_line.c
  C/render/geo/render_prism.c
  C/render/geo/render_linetexture.c
  C/render/geo/render_model.c
  C/render/geo/render_polygon_id_buffer.c
  C/render/geo/render_polygon_zfill.c
  C/render/geo/render_polygon_surface.c
  C/render/geo/render_polygon_texture_repeated.c
  C/render/geo/render_polygon_flat.c
  C/render/geo/clip_polygon.c
  C/render/geo/render_polygon.c
  C/render/surface.c
  
  C/render/palette.c

  #C/render/scanline.s
  
  C/physics/collide.c
  C/physics/raytrace.c
  
  C/object/object.c
  C/object/camera.c
  
  C/clipper/clipper.c
)

add_subdirectory(C/platform)  

add_library(X3D ${SRCS})

install(TARGETS X3D ARCHIVE DESTINATION /usr/local/lib)
install(DIRECTORY headers/ DESTINATION /usr/local/include/X3D FILES_MATCHING PATTERN "*")

