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

project(X3D-system-test CXX)

set(CMAKE_CXX_FLAGS "-g -std=c++14")

find_package(SDL REQUIRED)

set(TEST_SOURCE
    main.cpp
)

add_executable(system-tests ${TEST_SOURCE})

include_directories(/usr/local/include/X3D)
include_directories(/usr/local/include/X3D/system)

target_link_libraries(system-tests X3D-system ${SDL_LIBRARY})