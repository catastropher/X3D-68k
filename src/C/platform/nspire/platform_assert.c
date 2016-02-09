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

#include <stdlib.h>


#include <stdio.h>
#include <unistd.h>

#include "X3D_common.h"
#include "X3D_log.h"

void x3d_platform_assert_failed(const char* condition, const char* file, int16 line) {
  x3d_log(X3D_ERROR, "Assertion failed!\nFile: %s\nline: %d\n\t%s\n", file, line, condition);
  exit(0);
}

