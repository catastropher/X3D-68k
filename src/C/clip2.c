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

#include "X3D_common.h"
#include "X3D_vector.h"

typedef struct X3D_BoundRect {
  X3D_Vex2D start;
  X3D_Vex2D end;
} X3D_BoundRect;

enum {
  X3D_BOUNDRECT_INTERSECT = 0,
  X3D_BOUNDRECT_FAIL_LEFT = 1,
  X3D_BOUNDRECT_FAIL_RIGHT = 2,
  X3D_BOUNDRECT_FAIL_TOP = 4,
  X3D_BOUNDRECT_FAIL_BOTTOM = 8
};

///////////////////////////////////////////////////////////////////////////////
/// Determines whether two bounding rectangle intersect. If they don't,
///   this function also determines how b compares to a (i.e. whether
///   b is too far left of a).
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_boundrect_intersect(X3D_BoundRect* a, X3D_BoundRect* b) {
  uint16 flags = 0;
  
  if(b->end.x < a->start.x)
    flags |= X3D_BOUNDRECT_FAIL_LEFT;
  else if(b->start.x > a->end.x)
    flags |= X3D_BOUNDRECT_FAIL_RIGHT;

  if(b->end.y < a->start.y)
    flags |= X3D_BOUNDRECT_FAIL_TOP;
  else if(b->start.y > a->end.y)
    flags |= X3D_BOUNDRECT_FAIL_BOTTOM;
  
  return flags;
}

