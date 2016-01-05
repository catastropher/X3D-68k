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
X3D_UncompressedSegment x3d_segmentbuilder_add_uncompressed_segment(X3D_Prism3D* prism) {  
  X3D_UncompressedSegment* seg = x3d_segmentmanager_add(
    x3d_uncompressedsegment_size(prism->base_v));
  
  // Copy over the prism data
  x3d_prism3d_copy(&seg->prism, prism);
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new uncompressed segment that is not connected to anything else.
///
uint16 x3d_segmentbuilder_add_unconnected_segment(X3D_Prism3D* prism) {
  X3D_SegmentManager* segman = x3d_segmentmanager_get();
  uint16 total_segments = segman->alloc.alloc_offset.size;
  
  
}

uint16 x3d_segmentbuilder_add_extruded_segment(X3D_SegFaceID id, int16 dist) {
  
}

uint16 x3d_segmentbuilder_add_connecting_segment(X3D_SegFaceID a, X3D_SegFace b) {
  
}

