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
#include "X3D_plane.h"

typedef struct X3D_Line3D {
  X3D_Vex3D start;
  X3D_Vex3D dir;
} X3D_Line3D;

struct X3D_Polygon3D;

_Bool x3d_line3d_intersect_plane(X3D_Line3D* line, X3D_Plane* plane, X3D_Vex3D* inter, int16* scale);
_Bool x3d_line3d_intersect_polygon(X3D_Line3D* line, struct X3D_Polygon3D* poly, X3D_Vex3D* inter, int16* scale);

