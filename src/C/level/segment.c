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
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "memory/X3D_varsizeallocator.h"
#include "X3D_prism.h"
#include "X3D_trig.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes the segment manager.
///
/// @param max_segments   - maximum number of segments
/// @param seg_pool_size  - memory to set aside for allocating segments
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL void x3d_segmentmanager_init(uint16 max_segments, uint16 seg_pool_size) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  x3d_log(X3D_INFO, "Segment manager init");
  
  x3d_varsizeallocator_init(&seg_manager->alloc, max_segments, seg_pool_size);
  
  // Clear the cache
  uint16 i;
  
  for(i = 0; i < X3D_SEGMENT_CACHE_SIZE; ++i) {
    seg_manager->cache.entry[i].seg.base.id = X3D_SEGMENT_NONE;
  }
}

void x3d_segmentmanager_cleanup(void) {
  x3d_varsizeallocator_cleanup(&x3d_segmentmanager_get()->alloc);
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new segment.
///////////////////////////////////////////////////////////////////////////////
X3D_SegmentBase* x3d_segmentmanager_add(uint16 size) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  uint16 id;
  X3D_SegmentBase* seg = x3d_varsizeallocator_alloc(&seg_manager->alloc, size, &id);
  
  seg->id = id;
  seg->flags = 0;
  
  uint16 d;
  for(d = 0; d < X3D_MAX_OBJECTS_IN_SEG; ++d) {
    ((X3D_Segment* )seg)->object_list.objects[d] = X3D_INVALID_HANDLE;
  }
  
  return seg;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets a pointer to the interal representation of a segment.
///
/// @param id - ID of the segment to get
///
/// @return The addess of the interal segment representation.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL X3D_SegmentBase* x3d_segmentmanager_get_internal(uint16 id) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  return x3d_varsizeallocator_get(&seg_manager->alloc, id);
}


///////////////////////////////////////////////////////////////////////////////
/// Decompresses a segment and loads it into cache, if it's not already there.
///
/// @param id - id of the segment to load
///
/// @return The address of the uncompressed segment.
/// @note This address is only valid while the segment remains in cache!
/// @todo Implemented some sort of locking mechinism?
/// @todo Implement LRU strategy for deciding which block should be replaced.
///////////////////////////////////////////////////////////////////////////////
X3D_Segment* x3d_segmentmanager_load(uint16 id) {
  X3D_SegmentBase* seg = x3d_segmentmanager_get_internal(id);
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  // Calculate the plane equations for the faces
  uint16 d;
  X3D_Polygon3D poly = {
    .v = alloca(seg->base_v * sizeof(X3D_Vex3D))
  };
  
  X3D_Segment* useg = seg;
  
  X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(
    seg);
  
  //seg_manager->cache.entry[i].seg.last_engine_step = 0;
  
  for(d = 0; d < x3d_prism3d_total_f(seg->base_v); ++d) {
    x3d_prism3d_get_face(&useg->prism, d, &poly);
    x3d_plane_construct(&face[d].plane, poly.v, poly.v + 1, poly.v + 2);
    face[d].texture = x3d_uncompressedsegment_get_faces(((X3D_Segment *)seg))[d].texture;    
  }
  
  return seg;  
}

void x3d_uncompressedsegment_add_object(uint16 seg_id, X3D_Handle object) {
  _Bool exists = X3D_FALSE;
  uint16 free_pos = 0xFFFF;
  
  X3D_Segment* seg = x3d_segmentmanager_load(seg_id);
  
  uint16 i;
  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEG; ++i) {
    if(seg->object_list.objects[i] == X3D_INVALID_HANDLE)
      free_pos = i;
    else if(seg->object_list.objects[i] == object)
      exists = X3D_TRUE;
  }
  
  if(!exists) {
    // Can't fit any more objects in the room
    x3d_assert(free_pos != 0xFFFF);
    
    seg->object_list.objects[free_pos] = object;
    
    x3d_log(X3D_INFO, "Moved object %d to seg %d\n", object, seg_id);
  }
}

void x3d_segment_point_normal(X3D_Segment* seg, uint16 point, X3D_Vex3D* dest, X3D_Vex3D_int16* face_normal, angle256 angle) {
  uint16 p[3];
  X3D_Vex3D_int32 sum = { 0, 0, 0 };
  
  x3d_prism_point_faces(seg->prism.base_v, point, p);
  
  fp0x16 angle_cos = x3d_cos(ANG_30);
  
  X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  uint16 i;
  
  int16 total = 0;
  
  for(i = 0; i < 3; ++i) {
    X3D_Vex3D* normal = &face[p[i]].plane.normal;
    
    _Bool eq = face_normal->x == normal->x && face_normal->y == normal->y && face_normal->z == normal->z;
    
    if(x3d_vex3d_fp0x16_dot(normal, face_normal) > angle_cos || eq) {
      sum.x += normal->x;
      sum.y += normal->y;
      sum.z += normal->z;
      ++total;
    }
  }
  
  //x3d_assert(total != 0);
  
  //x3d_log(X3D_INFO, "total: %d", total);
  
  if(total != 0) {
    dest->x = sum.x / total;
    dest->y = sum.y / total;
    dest->z = sum.z / total;
  }
  else {
    *dest = *face_normal;
    x3d_log(X3D_INFO, "Case!");
  }
}


