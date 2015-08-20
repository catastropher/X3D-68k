/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#include "X3D_frustum.h"
#include "X3D_object.h"
#include "X3D_segment.h"
#include "X3D_vector.h"

typedef struct X3D_PlaneCollision {

} X3D_PlaneCollision;

_Bool x3d_point_in_segment(X3D_Segment* seg, Vex3D* p) {
  uint16 i;

  X3D_SegmentFace* face = x3d_segment_get_face(seg);

  for(i = 0; i < x3d_segment_total_f(seg); ++i) {
    printf("dist: %d\n", x3d_dist_to_plane(&face[i].plane, p));

    if(x3d_dist_to_plane(&face[i].plane, p) > 0)
      return FALSE;
  }

  return TRUE;
}

