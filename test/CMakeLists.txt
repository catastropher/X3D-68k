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
#   - USE_TILIBS - link in tilibs for multiplayer connections to the calc

cmake_minimum_required(VERSION 3.1)

project(xtest CXX)

if(NOT DEFINED XTARGET)
    set(XTARGET "pc")
endif()

if(NOT DEFINED X_LIB_PATH)
    set(X_LIB_PATH /usr/local/lib)
endif()

if(NOT DEFINED X_HEADER_PATH)
    set(X_HEADER_PATH /usr/local/include)
endif()

if(NOT DEFINED USE_TILIBS)
    set(USE_TILIBS "0")
endif()

if(${XTARGET} STREQUAL "pc")
    set(CMAKE_CXX_FLAGS "-fPIC -Wall -g -fsanitize=address -fsanitize=undefined -static-libasan -D_Bool=bool")
    #set(CMAKE_CXX_FLAGS "-std=gnu99 -Wall -g -O3")
    add_definitions(-D__pc__)
elseif(${XTARGET} STREQUAL "nspire")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")       # Workaround for CMake wanting to add -rdynamic
    set(CMAKE_C_COMPILER nspire-gcc)
    add_definitions(-D__nspire__)
    set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O3")
endif()

include_directories(${X_HEADER_PATH}/X3D)
include_directories(${X_HEADER_PATH})

add_executable(xtest
    src/init.cpp
    src/keys.cpp
    src/main.cpp
    src/render.cpp
    src/screen.cpp
    src/game.cpp
)

find_package(SDL REQUIRED)

find_library(X3D_LIBRARY X3D ${X_LIB_PATH})
target_link_libraries(xtest ${X3D_LIBRARY} ${SDL_LIBRARY} m)

if(${XTARGET} STREQUAL "pc")
    if(${USE_TILIBS} STREQUAL "1")
        message("Building with tilibs")
    
        find_package(PkgConfig REQUIRED)
        
        include_directories(/usr/local/include/tilp2)
        pkg_check_modules(GLIB2 REQUIRED glib-2.0)
        include_directories(${GLIB2_INCLUDE_DIRS})
        
        link_directories(${GLIB2_LIBRARY_DIRS})
        add_definitions(${GLIB2_CFLAGS_OTHER})
        
        link_directories(/usr/lib/tilp2)
        
        find_library(TICABLES ticables2 REQUIRED)
        find_library(TICALCS ticalcs2 REQUIRED)
            
        target_link_libraries(xtest ${GLIB2_LIBRARIES} ${TICALCS} ${TICABLES})
    endif()
endif()
