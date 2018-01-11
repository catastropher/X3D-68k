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

# Options:
#   - XTARGET - system to build for ("pc" or "nspire")
#   - X_LIB_PATH - location to install library (default /usr/local/lib)
#   - X_HEADER_PATH - location to install header files (default /usr/local/include)
#   - X_WITH_SDL - whether to use SDL as the video backend (automatically set for nspire and pc)
#   - USE_TILIBS - link in tilibs for multiplayer connections to the calc

cmake_minimum_required(VERSION 3.1)

project(X3D C ASM)

if(NOT DEFINED XTARGET)
    set(XTARGET "pc")
endif()

if(NOT DEFINED X_LIB_PATH)
    set(X_LIB_PATH "/usr/local/lib")
endif()

if(NOT DEFINED X_HEADER_PATH)
    set(X_HEADER_PATH "/usr/local/include")
endif()

if(NOT DEFINED USE_TILIBS)
    set(USE_TILIBS "0")
endif()

if(${XTARGET} STREQUAL "pc")
    set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -O2")
    #set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -O0 -g")
    
    set(X_WITH_SDL "1")
elseif(${XTARGET} STREQUAL "nspire")
    set(CMAKE_C_COMPILER nspire-gcc)
    set(CMAKE_ASM_COMPILER nspire-as)
    set(CMAKE_LI)
    add_definitions(-D__nspire__)
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O3")
    
    set(X_WITH_SDL "1")
endif()

include_directories(src/)

set(X_SOURCES
    # client
    src/client/X_Client.c

    # dev
    src/dev/X_AutoCompleter.c
    src/dev/X_conCommands.c
    src/dev/X_Console.c
    src/dev/X_DemoPlayer.c
    src/dev/X_DemoRecorder.c
    src/dev/X_TokenLexer.c

    # engine
    src/engine/X_engine.c
    src/engine/X_EngineContext.c
    src/engine/X_init.c

    # error
    src/error/X_error.c
    src/error/X_log.c

    # geo
    src/geo/X_BoundBox.c
    src/geo/X_Cube.c
    src/geo/X_Frustum.c
    src/geo/X_Plane.c
    src/geo/X_Polygon3.c
    src/geo/X_Ray3.c
    src/geo/X_Vec3.c

    # level
    src/level/X_BspLevel.c
    src/level/X_BspLevelLoader.c
    src/level/X_EntityModel.c
    src/level/X_EntityModelLoader.c
    src/level/X_RayTracer.c

    # math
    src/math/X_Mat4x4.c
    src/math/X_Quaternion.c
    src/math/X_sqrt.c
    src/math/X_trig.c

    # memory
    src/memory/X_alloc.c
    src/memory/X_Cache.c
    src/memory/X_Factory.c
    src/memory/X_String.c

    # net
    src/net/X_net.c
    
    # object
    src/object/X_ButtonObject.c
    src/object/X_CameraObject.c
    src/object/X_GameObject.c
    src/object/X_GameObjectLoader.c
    src/object/X_ObjectFactory.c
    src/object/X_PlatformObject.c
    src/object/X_WorldObject.c

    # physics
    src/physics/X_BoxCollider.c

    # render
    src/render/X_activeedge.c
    src/render/X_Font.c
    src/render/X_Palette.c
    src/render/X_Renderer.c
    src/render/X_Screen.c
    src/render/X_span.c
    src/render/X_Surface.c
    src/render/X_Texture.c
    src/render/X_TriangleFiller.c
    src/render/X_Viewport.c
    
    # server
    src/server/X_Server.c
    src/server/X_ServerPackets.c

    # system
    src/system/X_File.c
    src/system/X_Keys.c
    src/system/X_Mouse.c
    src/system/X_PackFile.c

    # util
    src/util/X_util.c
)

if(${XTARGET} STREQUAL "pc")
    set(X_SOURCES ${X_SOURCES}
        src/platform/platform_pc.c
    )
    
    if(${USE_TILIBS} STREQUAL "1")
        message("Building with tilibs")
        
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
        link_libraries(Threads::Threads)
        
        find_package(PkgConfig REQUIRED)
        
        include_directories(/usr/include/tilp2)
        pkg_check_modules(GLIB2 REQUIRED glib-2.0)
        include_directories(${GLIB2_INCLUDE_DIRS})
        
        set(X_SOURCES ${X_SOURCES}
            src/platform/socket_pc_to_nspire.c
        )
    endif()
endif()

if(${XTARGET} STREQUAL "nspire")
    set(X_SOURCES ${X_SOURCES}
        src/render/X_span_arm.s
        src/platform/platform_nspire.c
        src/platform/socket_nspire.c
    )
endif()



if(DEFINED X_WITH_SDL)
    find_package(SDL REQUIRED)
    include_directories(SDL_INCLUDE_DIR)
    
    set(X_SOURCES ${X_SOURCES} src/platform/X_SDL.c)
endif()

add_library(X3D STATIC ${X_SOURCES})

install(TARGETS X3D ARCHIVE DESTINATION ${X_LIB_PATH})
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D FILES_MATCHING PATTERN "*.h")
