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
#include "X3D_collide.h"
#include "X3D_plane.h"
#include "X3D_assert.h"
#include "X3D_vector.h"


void x3d_raycaster_init(X3D_RayCaster* caster, uint16 seg_id, X3D_Vex3D_fp16x8 pos, X3D_Vex3D_fp0x16 dir) {
  caster->seg = seg_id;
  caster->pos = pos;
  caster->dir = dir;
  caster->inside = X3D_TRUE;
}

void x3d_raycaster_cast(X3D_RayCaster* caster) {
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(caster->seg);
  
  /// @todo Add bounding sphere to segment cache
  /// Until then, a max radius of 500 is used
  X3D_BoundSphere temp_sphere = {
    .r = 2000,
  };
  
  X3D_BoundSphere* sphere = &temp_sphere;
  
  // Move the caster twice the radius of the current segment so that, no matter
  // what, it's guaranteed to be outside of the segment
  X3D_Vex3D_fp16x8 shift = {
    ((int32)sphere->r * 2 * caster->dir.x) >> 8,
    ((int32)sphere->r * 2 * caster->dir.y) >> 8,
    ((int32)sphere->r * 2 * caster->dir.z) >> 8
  };
  
  X3D_Vex3D in = { caster->pos.x >> 8, caster->pos.y >> 8, caster->pos.z >> 8 };
  
  //printf("In: %d, %d, %d\n", in.x, in.y, in.z);
  
  caster->pos.x += shift.x;
  caster->pos.y += shift.y;
  caster->pos.z += shift.z;
  
  X3D_Vex3D out = { caster->pos.x >> 8, caster->pos.y >> 8, caster->pos.z >> 8 };
  
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  
  // Find which of the segment faces the ray collides with, and more importantly,
  // which face we hit first. A portion of the line will be "inside" the plane
  // and a portion of the line will be "outside". For each plane, we calculate
  // this and whichever has the smallest (positive) percentage is the first one
  // hit.
  fp0x16 min_scale = 0x7FFF;
  int16 min_face = -1;
  int16 min_dist;
  
  int16 i;
  for(i = 0; i < x3d_prism3d_total_f(seg->base.base_v); ++i) {
    /// @todo dist_in can be moved into the below 'if'
    int16 dist_in = x3d_plane_dist(&face[i].plane, &in);
    int16 dist_out = x3d_plane_dist(&face[i].plane, &out);
    
    
    //printf("dddd %d: in %d out[%d]\n", i, dist_in, dist_out);
    
    if(dist_in <= 0) {
      caster->inside = X3D_FALSE;
      return;
    }
    
    // If the distance for the rayscast start is < 0, the point wasn't actually
    // inside the segment
    //x3d_assert(dist_in >= 0);
    
    // Only include faces that the caster is outside of
    if(dist_in > 0 && dist_out <= 0) {
      // Numerator/denominator of fraction for percentage calculation
      // percent in (aka scale) = dist_in / (dist_in + abs(dist_out))
      int16 n = dist_in;
      int16 d = dist_in - dist_out;   // dist_out < 0
      fp0x16 scale;
      
      if(n == d)
        scale = 0x7FFF;   // Prevents overflow (n == d results in 0x8000, too big for an int16)
      else
        scale = (n * 32768L) / d;   /// @todo Should be replaced with constant instead of magic number
        
        
      //x3d_assert(scale > 0);
        
      //if(scale < 0)
      //  printf("Neg scale!\n");
        
      if(scale < min_scale) {
        min_scale = scale;
        min_face = i;
        min_dist = dist_in;
      }
    }
  }
  
  printf("Min dist: %d\n", min_dist);
  
  
  // No face was actually hit...
  //x3d_assert(min_face != -1);
  if(min_face == -1) {
    x3d_log(X3D_INFO, "Segment: %d\n", caster->seg);
    x3d_assert(0);
  }
  
  
  // Calculate the point of intersection
  X3D_Vex3D diff = x3d_vex3d_sub(&out, &in);
  
  caster->hit_pos.x = in.x + (((int32)diff.x * min_scale) >> X3D_NORMAL_BITS);
  caster->hit_pos.y = in.y + (((int32)diff.y * min_scale) >> X3D_NORMAL_BITS);
  caster->hit_pos.z = in.z + (((int32)diff.z * min_scale) >> X3D_NORMAL_BITS);
  
  caster->hit_face = x3d_segfaceid_create(caster->seg, min_face);
  caster->dist = min_dist;
}



