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

project(xtest C)

if(NOT DEFINED XTARGET)
    set(X3D_TARGET "pc")
endif()

if(${XTARGET} STREQUAL "pc")
    set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -static-libasan")
elseif(${XTARGET} STREQUAL "nspire")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")       # Workaround for CMake wanting to add -rdynamic
    set(CMAKE_C_COMPILER nspire-gcc)
    add_definitions(-D__nspire__)
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2")
endif()

include_directories(/usr/local/include/X3D)
include_directories(/usr/local/include/)

add_executable(xtest
    src/keys.c
    src/main.c
    src/palette.c
    src/screen.c
)

find_package(SDL REQUIRED)

find_library(X3D_LIBRARY X3D /usr/local/lib)
target_link_libraries(xtest ${X3D_LIBRARY} SDL m)
