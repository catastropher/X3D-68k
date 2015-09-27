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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"
#include "X3D_clip.h"
#include "X3D_render.h"
#include "X3D_frustum.h"
#include "X3D_error.h"
#include "X3D_segment.h"
#include "X3D_prism.h"

typedef struct X3D_BoundLine {
  Vex2D normal;
  int16 d;
  Vex2D point;
} X3D_BoundLine;

void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
  
}

typedef struct X3D_BoundRegion {
  uint16 total_bl;
  X3D_BoundLine line[];
} X3D_BoundRegion;


void x3d_construct_boundregion(X3D_BoundRegion* region, Vex2D v[], uint16 total_v) {
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    x3d_construct_boundline(region->line + i, v + i, v + (i % total_v));
  }
  
  region->total_bl = total_v;
}

typedef struct X3D_EdgeClip {
  Vex2D a, b;
  _Bool a_clipped;
  _Bool b_clipped;
} X3D_EdgeClip;

#define DIST(_clip, _line, _v) _clip->dist[_line * _clip->region->total_bl + _v]

inline x3d_dist_to_line(X3D_BoundLine* line, Vex2D* v) {
  return (((int32)line->normal.x * v->x) + ((int32)line->normal.y * v->y)) >> X3D_NORMAL_SHIFT;
}

typedef struct X3D_ClipData {
  X3D_Segment* seg;
  X3D_BoundRegion* region;
  X3D_Prism2D* prism;
  int16* dist;
  uint16 total_v;
} X3D_ClipData;

inline x3d_calc_distances_to_lines(X3D_ClipData* clip) {
  uint16 line, vex;
  
  for(line = 0; line < clip->region->total_bl; ++line) {
    for(vex = 0; vex < clip->total_v; ++vex) {
      DIST(clip, line, vex) = x3d_dist_to_line(clip->region->line + line, clip->prism->v + vex);
    }
  }
}

inline void x3d_get_prism2d_edge(X3D_Prism2D* p, uint16 id, uint16* a, uint16* b) {
  if(id < p->base_v) {
    *a = id;
    
    if(id != p->base_v - 1)
      *b = id + 1;
    else
      *b = 0;
  }
  else if(id < p->base_v * 2) {
    *a = x3d_prism2d_opposite_vertex(p, id - p->base_v);
    
    if(id != p->base_v * 2 - 1)
      *b = x3d_prism2d_opposite_vertex(p, id + 1 - p->base_v);
    else
      *b = x3d_prism2d_opposite_vertex(p, p->base_v - p->base_v);
  }
  else {
    *a = x3d_prism2d_opposite_vertex(p, id - p->base_v * 2);
    *b = id - p->base_v * 2;
  }
}

typedef struct X3D_SegmentClip {
  uint16 total_e;
  X3D_EdgeClip edge[];
} X3D_SegmentClip;


void x3d_clip_render_segment_walls(X3D_Segment* seg, X3D_BoundRegion* region) {
  X3D_Prism2D* prism;
  uint16 total_v = prism->base_v * 2;
  
  int16 dist[region->total_bl * total_v];
  
  
  X3D_ClipData clip = {
    .seg = seg,
    .region = region,
    .prism = prism,
    .total_v = total_v,
    .dist = dist
  };
  
  
  //x3d_calc_distances_to_lines(region, prism->v, region->total_bl, total_v, dist);
  
  
}










































