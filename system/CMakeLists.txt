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

cmake_minimum_required(VERSION 3.1)

project(X3D-system CXX)

set(CMAKE_CXX_FLAGS, "-std=c++14")

include_directories(src/)

if(NOT DEFINED X_LIB_PATH)
    set(X_LIB_PATH "/usr/local/lib")
endif()

if(NOT DEFINED X_HEADER_PATH)
    set(X_HEADER_PATH "/usr/local/include")
endif()

set(SYSTEM_SOURCE
    # error
    src/error/X_OutOfMemoryException.cpp

    # instance
    src/instance/X_System.cpp
    src/instance/X_SystemInstance.cpp

    # log
    src/log/X_Log.cpp

    # memory
    src/memory/X_LinearAllocator.cpp
    src/memory/X_MemoryManager.cpp
    src/memory/X_SystemAllocator.cpp
    src/memory/X_ZoneAllocator.cpp
)

add_library(X3D-system STATIC ${SYSTEM_SOURCE})

install(TARGETS X3D-system ARCHIVE DESTINATION ${X_LIB_PATH})
install(DIRECTORY src/ DESTINATION ${X_HEADER_PATH}/X3D/system FILES_MATCHING PATTERN "*.hpp")

install(FILES "src/X3D-System.hpp" DESTINATION ${X_HEADER_PATH}/X3D)