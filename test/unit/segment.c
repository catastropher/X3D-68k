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

#include <alloca.h>

#include "X3D.h"

void test_segment(void) {
  uint16 base_v = 8;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };

  x3d_prism3d_construct(prism, base_v, 50, 100, angle);
  
  X3D_Segment* seg = x3d_segmentbuilder_add_uncompressed_segment(prism);
  X3D_Segment* r_seg = (X3D_Segment* )x3d_segmentmanager_get_internal(seg->base.id);
  
  x3d_assert(seg == r_seg);
  x3d_assert(seg->base.base_v == base_v);
  
  X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  uint16 total_f = x3d_prism3d_total_f(prism->base_v);
  
  x3d_assert(total_f == 10);
  x3d_assert(face[9].portal_seg_face == X3D_FACE_NONE);
  
  x3d_segmentmanager_load(0);
  x3d_segmentmanager_load(0);
  
  
  
}

