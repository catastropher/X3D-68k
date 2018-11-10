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

cmake_minimum_required(VERSION 3.5)

project(x3d-system LANGUAGES CXX)

set(SOURCE
    # filesystem
    src/filesystem/X_FileHandle.cpp
    src/filesystem/X_FilePath.cpp
    src/filesystem/X_FileReader.cpp
    src/filesystem/X_FileSystem.cpp
    src/filesystem/X_Pak.cpp
    src/filesystem/X_PakWriter.cpp

    # instance
    src/instance/X_System.cpp

    # keys
    src/keys/Sdl1KeyboardDriver.cpp

    # log
    src/log/X_Log.cpp

    # memory
    src/memory/X_Cache.cpp
    src/memory/X_Crc32.cpp
    src/memory/X_LinearAllocator.cpp
    src/memory/X_MemoryManager.cpp
    src/memory/X_SystemAllocator.cpp
    src/memory/X_ZoneAllocator.cpp

    # screen
    src/screen/X_Palette.cpp

    # service
    src/service/X_Services.cpp
)

find_package(SDL REQUIRED)

add_library(x3d-system STATIC ${SOURCE})

set_target_properties(x3d-system PROPERTIES EXPORT_NAME system)

#Set target properties
target_include_directories(x3d-system
    PUBLIC
        $<INSTALL_INTERFACE:include/system>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_options(x3d-system PRIVATE -Wall -std=c++14)

##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/X3D)

install(TARGETS x3d-system
    EXPORT x3d-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/X3D COMPONENT shlib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/X3D
)

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/X3D/system FILES_MATCHING PATTERN "*.hpp")

