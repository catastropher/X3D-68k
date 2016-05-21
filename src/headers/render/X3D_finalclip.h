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

#pragma once

#include "X3D_common.h"

typedef struct X3D_Line2D {
  int16 a;
  int16 b;
  int16 c;
} X3D_Line2D;

typedef struct X3D_ClipBucket {
  int16 min_y;
  int16 max_y;
  int16 x_left;
  int16 x_right;
  
  X3D_Line2D left;
  X3D_Line2D right;
} X3D_ClipBucket;

typedef uint16 x3d_clipedge_t;
typedef uint16 x3d_clipbucket_t;
typedef uint16 x3d_clipbucket_edge_t;

typedef uint16 x3d_vertex_status_t;
typedef uint16 x3d_vertex_t;

enum {
  X3D_POINT_INSIDE = 0,
  X3D_POINT_OUT_LEFT = 1,
  X3D_POINT_OUT_RIGHT = 2,
  X3D_POINT_OUT_TOP = 4,
  X3D_POINT_OUT_BOTTOM = 8
};

typedef struct X3D_ClipVertex {
  x3d_clipbucket_t bucket;
  int16 dist_left;
  int16 dist_right;
  x3d_vertex_status_t status;
} X3D_ClipVertex;

typedef struct X3D_ClipEdge {
  x3d_clipbucket_edge_t edge[2];
  fp0x16                scale[2];
} X3D_ClipEdge;


typedef struct X3D_FinalClipContext {
  X3D_Vex2D* v;
  uint16 total_v;
  
  X3D_Pair* edges;
  uint16 total_e;
  
  X3D_ClipVertex* clip_v;
  X3D_ClipEdge* clip_e;
  
  X3D_ClipBucket* bucket;
  uint16 total_b;
  
  int16 min_y;
  int16 max_y;
} X3D_FinalClipContext;

