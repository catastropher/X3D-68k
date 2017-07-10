# This file is part of X3D.
#
# X3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# X3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with X3D. If not, see <http:#www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)

project(X3D C)

if(NOT DEFINED XTARGET)
    set(X3D_TARGET "pc")
endif()

if(${XTARGET} STREQUAL "pc")
    set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined")
elseif(${XTARGET} STREQUAL "nspire")
    set(CMAKE_C_COMPILER nspire-gcc)
    set(CMAKE_LI)
    add_definitions(-D__nspire__)
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2")
endif()

include_directories(src/)

add_library(X3D STATIC
    # dev
    src/dev/X_conCommands.c
    src/dev/X_Console.c

    # engine
    src/engine/X_engine.c
    src/engine/X_EngineContext.c

    # error
    src/error/X_error.c
    src/error/X_log.c
    
    # geo
    src/geo/X_Cube.c
    src/geo/X_Frustum.c
    src/geo/X_Plane.c
    src/geo/X_Polygon3.c
    src/geo/X_Ray3.c
    src/geo/X_Vec3.c

    # level
    src/level/X_BspLevel.c
    src/level/X_BspLevelLoader.c
    
    # math
    src/math/X_Mat4x4.c
    src/math/X_Quaternion.c
    src/math/X_sqrt.c
    src/math/X_trig.c
    
    # memory
    src/memory/X_alloc.c
    src/memory/X_Factory.c
    src/memory/X_String.c
    
    # object
    src/object/X_CameraObject.c
    src/object/X_CubeObject.c
    src/object/X_GameObject.c
    
    # render
    src/render/X_activeedge.c
    src/render/X_Canvas.c
    src/render/X_Font.c
    src/render/X_Palette.c
    src/render/X_Renderer.c
    src/render/X_Screen.c
    src/render/X_Texture.c
    src/render/X_Viewport.c
    
    # system
    src/system/X_File.c
    src/system/X_Keys.c
    src/system/X_PackFile.c
    
    # util
    src/util/X_util.c
)

install(TARGETS X3D ARCHIVE DESTINATION /usr/local/lib)
install(DIRECTORY src/ DESTINATION /usr/local/include/X3D FILES_MATCHING PATTERN "*.h")
