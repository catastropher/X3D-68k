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
#include "X3D_segmentbuilder.h"
#include "X3D_enginestate.h"

///////////////////////////////////////////////////////////////////////////////
X3D_UncompressedSegment* x3d_segmentbuilder_add_uncompressed_segment(X3D_Prism3D* prism) {  
  X3D_UncompressedSegment* seg = (X3D_UncompressedSegment*)x3d_segmentmanager_add(
    x3d_uncompressedsegment_size(prism->base_v));
  
  seg->face_offset = x3d_uncompressedsegment_face_offset(prism->base_v);
  
  // Copy over the prism data
  x3d_prism3d_copy(&seg->prism, prism);
  
  // Initialize all faces to have nothing connected to them
  uint16 i;
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  uint16 total_f = x3d_prism3d_total_f(prism->base_v);
  
  for(i = 0; i < total_f; ++i) {
    face[i].portal_seg_face = X3D_FACE_NONE;
  }
  
  seg->base.flags = X3D_SEGMENT_UNCOMPRESSED;
  
  return seg;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new uncompressed segment that is not connected to anything else.
///
uint16 x3d_segmentbuilder_add_unconnected_segment(X3D_Prism3D* prism) {
  return x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
}

uint16 x3d_segmentbuilder_add_extruded_segment(X3D_SegFaceID id, int16 dist) {
  
}

uint16 x3d_segmentbuilder_add_connecting_segment(X3D_SegFaceID a, X3D_SegFaceID b) {
  
}

