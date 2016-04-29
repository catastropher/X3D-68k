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
/// Creates a new uncompressed segment that can be edited and joined with
///   other segments.
///
/// @param prism - prism that describes the geometry of the segment
///
/// @return A pointer to the uncompressed segment.
/// @note All faces of the segment are set to not be connected to anything.
/// @note For internal use only.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL X3D_Segment* x3d_segmentbuilder_add_uncompressed_segment(X3D_Prism3D* prism) {  
  X3D_Segment* seg = (X3D_Segment*)x3d_segmentmanager_add(x3d_uncompressedsegment_size(prism->base_v));
  
  seg->face_offset = x3d_uncompressedsegment_face_offset(prism->base_v);
  
  // Copy over the prism data
  x3d_prism3d_copy(&seg->prism, prism);
  
  
  seg->base.flags = 0;
  seg->base.base_v = prism->base_v;
  
  x3d_segment_reset(seg);
  
  return seg;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new uncompressed segment that is not connected to anything else.
///
/// @param prism  - the prism that describes the geometry of the segment
///
/// @return The id of the uncompressed segment.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_segmentbuilder_add_unconnected_segment(X3D_Prism3D* prism) {
  return x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
}

uint16 x3d_segmentbuilder_add_extruded_segment(X3D_SegFaceID id, int16 dist) {
  uint16 seg_id = x3d_segfaceid_seg(id);
  uint16 face_id = x3d_segfaceid_face(id);
  
  /// @todo This implementation will only work uncompressed segments. Fix this!
  X3D_Segment* seg = (X3D_Segment*)x3d_segmentmanager_get_internal(seg_id);
  X3D_Polygon3D poly = {
    .v = alloca(sizeof(X3D_Vex3D) * seg->base.base_v)
  };
  
  // Extrude the face along its (flipped) normal
  X3D_Plane plane;
  x3d_prism3d_get_face(&seg->prism, face_id, &poly);
  x3d_polygon3d_calculate_plane(&poly, &plane);
  plane.normal = x3d_vex3d_neg(&plane.normal);
  
  X3D_Prism3D* new_prism = alloca(x3d_prism3d_size(poly.total_v));
  new_prism->base_v = poly.total_v;
  
  // The old faces becomes BASE_A of the new prism. But, if the old face was
  // BASE_B, it needs to be reversed (BASE_B is always reversed after calling
  // x3d_prism3d_get_face()).
  //if(face_id == X3D_BASE_A || face_id == X3D_BASE_B) {
    x3d_polygon3d_reverse(&poly);
  //}
  
  x3d_prism3d_set_face(new_prism, X3D_BASE_A, &poly);
  
  // BASE_B of the new prism becomes the translated polygon (but it has to be
  // because x3d_prism3d_set_face() expects BASE_B to be reversed).
  x3d_polygon3d_reverse(&poly);
  x3d_polygon3d_translate_normal(&poly, &plane.normal, dist);
  x3d_prism3d_set_face(new_prism, X3D_BASE_B, &poly);
  
  // Create a new segment with the new prism
  X3D_Segment* new_seg = x3d_segmentbuilder_add_uncompressed_segment(
    new_prism);
  
  X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  X3D_SegmentFace* new_face = x3d_uncompressedsegment_get_faces(new_seg);
  
  // Add the face portal connecting the two segments
  face[face_id].portal_seg_face = x3d_segfaceid_create(new_seg->base.id, X3D_BASE_A);
  new_face[X3D_BASE_A].portal_seg_face = id;
  
  //x3d_segmentmanager_cache_purge();
  
  return new_seg->base.id;
}

uint16 x3d_segmentbuilder_add_connecting_segment(X3D_SegFaceID a, X3D_SegFaceID b) {
  X3D_Segment* seg_a = x3d_segmentmanager_load(x3d_segfaceid_seg(a));
  X3D_Segment* seg_b = x3d_segmentmanager_load(x3d_segfaceid_seg(b));
  
  X3D_Polygon3D face_a = { .v = alloca(1000) };
  x3d_prism3d_get_face(&seg_a->prism, x3d_segfaceid_face(a), &face_a);
  
  X3D_Polygon3D face_b = { .v = alloca(1000) };
  x3d_prism3d_get_face(&seg_b->prism, x3d_segfaceid_face(b), &face_b);
  
  x3d_assert(face_a.total_v == face_b.total_v);
  
  X3D_Prism3D* new_prism = alloca(1000);
  
  new_prism->base_v = face_a.total_v;
  
  x3d_polygon3d_reverse(&face_a);
  x3d_polygon3d_reverse(&face_b);
  
  x3d_prism3d_set_face(new_prism, X3D_BASE_A, &face_a);
  x3d_prism3d_set_face(new_prism, X3D_BASE_B, &face_b);
  
  X3D_Segment* new_seg = x3d_segmentbuilder_add_uncompressed_segment(new_prism);
  
  x3d_uncompressedsegment_get_faces(seg_a)[x3d_segfaceid_face(a)].portal_seg_face = x3d_segfaceid_create(new_seg->base.id, X3D_BASE_A);
  x3d_uncompressedsegment_get_faces(new_seg)[X3D_BASE_A].portal_seg_face = a;
  
  x3d_uncompressedsegment_get_faces(seg_b)[x3d_segfaceid_face(b)].portal_seg_face = x3d_segfaceid_create(new_seg->base.id, X3D_BASE_B);
  x3d_uncompressedsegment_get_faces(new_seg)[X3D_BASE_B].portal_seg_face = b;
  
  return new_seg->base.id;
}

void x3d_segment_make_door(uint16 id) {
  X3D_Segment* seg = x3d_segmentmanager_load(id);
  
  seg->base.flags |= X3D_SEGMENT_DOOR;
  
  // Initialize the door
  seg->door_data = x3d_slab_alloc(sizeof(X3D_SegmentDoorData));
  seg->door_data->door_open = X3D_DOOR_CLOSED;
 
  
  
}



