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

#include "X3D_common.h"

static inline int32 x3d_file_read_int32(FILE* file) {
    int val;
    fscanf(file, "%d", &val);
    return val;
}

static inline X3D_Vex3D x3d_file_read_vex3d(FILE* file) {
    int x, y, z;
    fscanf(file, "%d %d %d", &x, &y, &z);
    
    return x3d_vex3d_make(x, y, z);
}

