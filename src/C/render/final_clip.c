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

#include "render/X3D_finalclip.h"
#include "X3D_vector.h"
#include "X3D_assert.h"

int32 x3d_line2d_dist(X3D_Line2D* line, X3D_Vex2D* v) {
  return (int32)line->a * v->x + (int32)line->b * v->y + line->c;
}

_Bool x3d_line2d_vertex_outside(int16 dist) {
  return dist < 0;
}

// Checks if a point is in the y range of a bucket
_Bool x3d_clipbucket_point_in_y_range(X3D_ClipBucket* bucket, X3D_Vex2D* v) {
  return v->y >= bucket->min_y && v->y < bucket->max_y;
}

// Checks if a point (that has a y of bucket->min_y) is inside the bucket
x3d_vertex_status_t x3d_clipbucket_point_in_x_range(X3D_ClipBucket* bucket, X3D_Vex2D* v) {
  if(v->x < bucket->x_left)
    return X3D_POINT_OUT_LEFT;
  
  if(v->x > bucket->x_right)
    return X3D_POINT_OUT_RIGHT;
  
  return X3D_POINT_INSIDE;
}

x3d_vertex_status_t x3d_clipbucket_point_inside(X3D_ClipBucket* bucket, X3D_Vex2D* v, X3D_ClipVertex* clip_v) {
  clip_v->dist_left = x3d_line2d_dist(&bucket->left, v);
  clip_v->dist_right = x3d_line2d_dist(&bucket->right, v);
  
  if(x3d_line2d_vertex_outside(clip_v->dist_left))
    return X3D_POINT_OUT_LEFT;
  
  if(x3d_line2d_vertex_outside(clip_v->dist_right))
    return X3D_POINT_OUT_RIGHT;
  
  return X3D_POINT_INSIDE;
}

// Finds which bucket a vertex belongs to
void x3d_clip_find_vertex_bucket(X3D_FinalClipContext* context, x3d_vertex_t v_index) {
  X3D_Vex2D* v = context->v + v_index;
  X3D_ClipVertex* clip_v = context->clip_v + v_index;
  
  if(v->y < context->min_y) {
    clip_v->status = X3D_POINT_OUT_TOP;
    return;
  }
  
  if(v->y > context->max_y) {
    clip_v->status = X3D_POINT_OUT_BOTTOM;
    return;
  }
  
  uint16 i;
  for(i = 0; i < context->total_b; ++i) {
    if(x3d_clipbucket_point_in_y_range(context->bucket + i, v)) {
      clip_v->status = x3d_clipbucket_point_inside(context->bucket + i, v, clip_v);
    }
  }
}

// Classifies each point into the bucket it's contained in
void x3d_clip_classify_v(X3D_FinalClipContext* context) {
  x3d_vertex_t i = 0;
  
  for(i = 0; i < context->total_v; ++i)
    x3d_clip_find_vertex_bucket(context, i);
}

