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

///////////////////////////////////////////////////////////////////////////////
/// A bounding rectangle. v[0] is the top left coordinate and v[1] is the
/// bottom right.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_BoundingRect {
  X3D_Vex2D v[2];
} X3D_BoundingRect;

//static inline x3d_boundingrect_init(X3D_BoundingRect* rect)

