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

project(X3D VERSION 1.0.0 LANGUAGES CXX)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/X3D/cmake)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/X3DConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/X3DConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/X3DConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# #Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/X3DConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/X3DConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)
add_subdirectory(engine)
add_subdirectory(graphics)
add_subdirectory(math)
add_subdirectory(system)

export(EXPORT x3d-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/X3DTargets.cmake NAMESPACE X3D::)
