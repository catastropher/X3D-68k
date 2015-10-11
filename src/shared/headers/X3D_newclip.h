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

#include "X3D_fix.h"
#include "X3D_vector.h"

#pragma once

typedef struct X3D_BoundLine {
  Vex2D normal;
  int16 d;
  Vex2D point;
} X3D_BoundLine;

typedef struct X3D_BoundRegion {
  uint16 total_bl;
  Vex2D point_inside;
  X3D_BoundLine line[];
} X3D_BoundRegion;

typedef struct X3D_EdgeClip {
  uint16 v[2];
  uint16 clip_status[2];
} X3D_EdgeClip;

enum {
  CLIP_INVISIBLE,
  CLIP_VISIBLE,
  CLIP_CLIPPED
};

typedef struct X3D_IndexedEdge {
  uint16 v[2];
} X3D_IndexedEdge;

typedef struct X3D_ClippedVertex {
  Vex2D v;
  uint16 clip_status;
  uint16 clip_line;
} X3D_ClippedVertex;

typedef struct X3D_ClippedEdge {
  X3D_ClippedVertex v[2];
} X3D_ClippedEdge;


typedef struct X3D_ClipData {
  X3D_BoundRegion*  region;
  Vex2D*            v;
  uint16            total_v;
  X3D_IndexedEdge*  edge;
  uint16            total_e;
  X3D_ClippedEdge*  edge_clip;
  
  int16*            line_dist;
  uint16*           outside;
  uint16*           outside_total;
  uint32*           outside_mask;
} X3D_ClipData;

void x3d_test_new_clip();

