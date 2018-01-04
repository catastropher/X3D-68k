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
    set(XTARGET "pc")
endif()

if(NOT DEFINED X_LIB_PATH)
    set(X_LIB_PATH /usr/local/lib)
endif()

if(NOT DEFINED X_HEADER_PATH)
    set(X_HEADER_PATH /usr/local/include)
endif()

if(${XTARGET} STREQUAL "pc")
    #set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -static-libasan")
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -g")
elseif(${XTARGET} STREQUAL "nspire")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")       # Workaround for CMake wanting to add -rdynamic
    set(CMAKE_C_COMPILER nspire-gcc)
    add_definitions(-D__nspire__)
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O3")
endif()

include_directories(${X_HEADER_PATH}/X3D)
include_directories(${X_HEADER_PATH})

add_executable(xtest
    src/init.c
    src/keys.c
    src/main.c
    src/render.c
    src/screen.c
)

find_package(SDL REQUIRED)

find_library(X3D_LIBRARY X3D ${X_LIB_PATH})
target_link_libraries(xtest ${X3D_LIBRARY} SDL m)


if(${XTARGET} STREQUAL "pc")
    find_package(PkgConfig REQUIRED)
    
    include_directories(/usr/local/include/tilp2)
    pkg_check_modules(GLIB2 REQUIRED glib-2.0)
    include_directories(${GLIB2_INCLUDE_DIRS})
    
    link_directories(${GLIB2_LIBRARY_DIRS})
    add_definitions(${GLIB2_CFLAGS_OTHER})
    
    link_directories(/usr/lib/tilp2)
    
    find_library(TICABLES ticables2)
    find_library(TICALCS ticalcs2)
        
    target_link_libraries(xtest ${GLIB2_LIBRARIES} ${TICALCS} ${TICABLES})
endif()
    
