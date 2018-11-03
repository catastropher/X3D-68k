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

project(x3d-graphics LANGUAGES CXX)

set(SOURCE
    src/Canvas.cpp
    src/Texture.cpp
)

add_library(x3d-graphics STATIC ${SOURCE})

set_target_properties(x3d-graphics PROPERTIES EXPORT_NAME graphics)

#Set target properties
target_include_directories(x3d-graphics
    PUBLIC
        $<INSTALL_INTERFACE:include/graphics>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_options(x3d-graphics PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -std=c++14>)

# Dependencies

target_link_libraries(x3d-graphics x3d-math x3d-system)


##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/X3D)

install(TARGETS x3d-graphics
    EXPORT x3d-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/X3D COMPONENT shlib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/X3D
)

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/X3D/graphics FILES_MATCHING PATTERN "*.hpp")

