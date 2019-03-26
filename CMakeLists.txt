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
    set(CMAKE_CXX_FLAGS "-std=c++14 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -O2 -Wno-unused-result -rdynamic")
    #set(CMAKE_CXX_FLAGS "-std=gnu99 -fPIC -Wall -O3 -g")
    
    set(X_WITH_SDL "1")
elseif(${XTARGET} STREQUAL "nspire")
    set(CMAKE_C_COMPILER nspire-gcc)
    set(CMAKE_CXX_COMPILER nspire-gcc)
    set(CMAKE_ASM_COMPILER nspire-as)
    set(CMAKE_LI)
    add_definitions(-D__nspire__)
    set(CMAKE_CXX_FLAGS "-Wall -O3")
    
    set(X_WITH_SDL "1")
endif()

include_directories(src/)

set(X_SOURCES
    # dev
        src/dev/console/AutoCompleter.cpp
        src/dev/console/DefaultCommands.cpp
        src/dev/console/Console.cpp
        src/dev/console/TokenLexer.cpp

    # dev/console
        src/dev/console/ConsoleRenderer.cpp

        # engine
        src/engine/Engine.cpp
        src/engine/EngineContext.cpp
        src/engine/EngineQueue.cpp
        src/engine/Init.cpp
    
    # entity
        # builtin
            src/entity/builtin/WorldEntity.cpp
            src/entity/builtin/DoorEntity.cpp

        src/entity/component/BrushModelComponent.cpp
        src/entity/EntityDictionary.cpp
        src/entity/EntityDictionaryParser.cpp
        src/entity/EntityManager.cpp
        src/entity/EntityBuilder.cpp

        # error
        src/error/Error.cpp
        src/error/Log.cpp

    # geo
        src/geo/BoundBox.cpp
        src/geo/Frustum.cpp
        src/geo/Plane.cpp
        src/geo/Polygon3.cpp
        src/geo/Ray3.cpp
        src/geo/Vec3.cpp

    # level
    src/level/BrushModelBuilder.cpp
        src/level/BspLevel.cpp
        src/level/BspLevelLoader.cpp
    src/level/BspModel.cpp
        src/level/BspNode.cpp
        src/level/BspRayTracer.cpp
        src/level/CollisionHullBuilder.cpp
        src/level/EntityModel.cpp
        src/level/EntityModelLoader.cpp
        src/level/Portal.cpp
    src/level/PotentiallyVisibleSet.cpp

    # math
        src/math/Mat4x4.cpp
        src/math/Quaternion.cpp
        src/math/FastSqrt.cpp
        src/math/Trig.cpp

    # memory
        src/memory/Alloc.cpp
        src/memory/Cache.cpp
        src/memory/Memory.cpp
        src/memory/String.cpp

    # object
        src/render/Camera.cpp

    # physics
    src/physics/PhysicsEngine.cpp
        src/physics/BoxColliderEngine.cpp
        src/physics/BoxColliderMoveLogic.cpp

    # render
    src/render/WireframeLevelRenderer.cpp
        src/render/ActiveEdge.cpp
        src/render/Font.cpp
        src/render/Palette.cpp
        src/render/OldRenderer.cpp
        src/render/Screen.cpp
        src/render/Span.cpp
        src/render/Surface.cpp
        src/render/StatusBar.cpp
        src/render/Texture.cpp
        src/render/Viewport.cpp

    # system
    src/system/Clock.cpp
        src/system/File.cpp
        src/system/FilePath.cpp
        src/system/FileReader.cpp
        src/system/FileSystem.cpp
        src/system/Keys.cpp
        src/system/Mouse.cpp
        src/system/PackFile.cpp

    # util
        src/util/Json.cpp
    src/util/X_JsonParser.cpp
        src/util/StopWatch.cpp
    src/util/X_util.cpp
)

if(${XTARGET} STREQUAL "pc")
    set(X_SOURCES ${X_SOURCES}
        src/platform/platform_pc.cpp
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
            src/render/span-arm.s
        src/platform/platform_nspire.cpp
            src/platform/nspire/NspireScreenDriver.cpp
    )
endif()



if(DEFINED X_WITH_SDL)
    find_package(SDL REQUIRED)
    include_directories(SDL_INCLUDE_DIR)
    
    set(X_SOURCES ${X_SOURCES}
            src/platform/SDL.cpp
            src/platform/SDL/SdlScreenDriver.cpp
            src/entity/EntityDictionary.hpp src/entity/EntityDictionaryParser.cpp src/entity/EntityDictionaryParser.hpp src/entity/EntityDictionary.cpp src/level/LevelManager.hpp src/level/LevelManager.cpp src/entity/component/InputComponent.hpp src/memory/FixedSizeArray.hpp src/render/software/SoftwareRenderer.cpp src/render/software/SoftwareRenderer.hpp src/render/software/LevelRenderer.cpp src/render/software/LevelRenderer.hpp src/entity/EntityEvent.hpp src/memory/StringId.hpp src/memory/Crc32.hpp src/memory/Crc32.cpp src/entity/component/ComponentType.hpp src/memory/GroupAllocator.cpp src/memory/GroupAllocator.hpp src/entity/component/TransformComponent.cpp src/entity/system/IEntitySystem.hpp src/engine/GlobalConfiguration.hpp src/memory/Set.hpp src/entity/system/BrushModelSystem.hpp src/entity/system/CameraSystem.hpp src/entity/system/BoxColliderSystem.hpp src/entity/system/GenericComponentSystem.hpp src/util/StackTrace.cpp src/util/StackTrace.hpp src/entity/component/ScriptableComponent.cpp src/entity/component/ScriptableComponent.hpp src/hud/MessageQueue.cpp src/hud/MessageQueue.hpp src/entity/component/PhysicsComponent.hpp src/entity/builtin/TriggerEntity.cpp src/entity/builtin/TriggerEntity.hpp src/entity/system/PhysicsSystem.cpp src/entity/system/PhysicsSystem.hpp)
endif()

add_library(X3D STATIC ${X_SOURCES})

install(TARGETS X3D ARCHIVE DESTINATION ${X_LIB_PATH})
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D FILES_MATCHING PATTERN "*.h")
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D FILES_MATCHING PATTERN "*.hpp")
