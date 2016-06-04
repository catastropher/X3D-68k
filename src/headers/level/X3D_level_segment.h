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

#include "X3D_common.h"
#include "X3D_level_types.h"

struct X3D_Prism3D;

X3D_LEVEL_SEG x3d_level_segment_add(X3D_Level* level, struct X3D_Prism3D* prism, uint16 flags);

#ifdef X3D_LEVEL_SEGMENT_C

static void x3d_level_segment_array_expand(X3D_Level* level);

#endif

