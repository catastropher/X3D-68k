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

set(CMAKE_C_FLAGS "-std=gnu99 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -lasan")

project(xtest C)

include_directories(/usr/local/include/X3D)

add_executable(xtest
    src/keys.c
    src/main.c
    src/palette.c
    src/screen.c
)

find_package(SDL REQUIRED)

target_link_libraries(xtest X3D SDL)