// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <stdio.h>
#include <stdlib.h>

#define x_assert(_cond, message...) x_assert_function(_cond, __FILE__, __LINE__, message)

void x_assert_function(_Bool condition, const char* file, int line, const char* messageFormat, ...);
void x_system_error(const char* format, ...);

