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

project(X3D CXX ASM)

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
    set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -O2")
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
    src/client/X_Client.cpp

    # dev
    src/dev/X_AutoCompleter.cpp
    src/dev/X_conCommands.cpp
    src/dev/X_Console.cpp
    src/dev/X_DemoPlayer.cpp
    src/dev/X_DemoRecorder.cpp
    src/dev/X_TokenLexer.cpp

    # engine
    src/engine/X_engine.cpp
    src/engine/X_EngineContext.cpp
    src/engine/X_init.cpp

    # error
    src/error/X_error.cpp
    src/error/X_log.cpp

    # geo
    src/geo/X_BoundBox.cpp
    src/geo/X_Cube.cpp
    src/geo/X_Frustum.cpp
    src/geo/X_Plane.cpp
    src/geo/X_Polygon3.cpp
    src/geo/X_Ray3.cpp
    src/geo/X_Vec3.cpp

    # level
    src/level/X_BspLevel.cpp
    src/level/X_BspLevelLoader.cpp
    src/level/X_EntityModel.cpp
    src/level/X_EntityModelLoader.cpp
    src/level/X_RayTracer.cpp

    # math
    src/math/X_Mat4x4.cpp
    src/math/X_Quaternion.cpp
    src/math/X_sqrt.cpp
    src/math/X_trig.cpp

    # memory
    src/memory/X_alloc.cpp
    src/memory/X_Cache.cpp
    src/memory/X_Factory.cpp
    src/memory/X_String.cpp

    # net
    src/net/X_net.cpp
    
    # object
    src/object/X_ButtonObject.cpp
    src/object/X_CameraObject.cpp
    src/object/X_GameObject.cpp
    src/object/X_GameObjectLoader.cpp
    src/object/X_ObjectFactory.cpp
    src/object/X_PlatformObject.cpp
    src/object/X_WorldObject.cpp

    # physics
    src/physics/X_BoxCollider.cpp

    # render
    src/render/new-renderer.cpp
    src/render/X_activeedge.cpp
    src/render/X_Font.cpp
    src/render/X_Palette.cpp
    src/render/X_Renderer.cpp
    src/render/X_Screen.cpp
    src/render/X_span.cpp
    src/render/X_Surface.cpp
    src/render/X_Texture.cpp
    src/render/X_TriangleFiller.cpp
    src/render/X_Viewport.cpp
    
    # server
    src/server/X_Server.cpp
    src/server/X_ServerPackets.cpp

    # system
    src/system/X_File.cpp
    src/system/X_Keys.cpp
    src/system/X_Mouse.cpp
    src/system/X_PackFile.cpp

    # util
    src/util/X_util.cpp
)

if(${XTARGET} STREQUAL "pc")
    set(X_SOURCES ${X_SOURCES}
        src/platform/platform_pc.cpp
        src/platform/socket_pc.cpp
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
            src/platform/socket_pc_to_nspire.cpp
        )
    endif()
endif()

if(${XTARGET} STREQUAL "nspire")
    set(X_SOURCES ${X_SOURCES}
        src/render/X_span_arm.s
        src/platform/platform_nspire.cpp
        src/platform/socket_nspire.cpp
    )
endif()



if(DEFINED X_WITH_SDL)
    find_package(SDL REQUIRED)
    include_directories(SDL_INCLUDE_DIR)
    
    set(X_SOURCES ${X_SOURCES} src/platform/X_SDL.cpp)
endif()

add_library(X3D STATIC ${X_SOURCES})

install(TARGETS X3D ARCHIVE DESTINATION ${X_LIB_PATH})
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D FILES_MATCHING PATTERN "*.h")
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D FILES_MATCHING PATTERN "*.hpp")
