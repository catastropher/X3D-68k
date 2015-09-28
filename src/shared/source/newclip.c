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
#include "X3D_engine.h"

typedef struct X3D_BoundLine {
  Vex2D normal;
  int16 d;
  Vex2D point;
} X3D_BoundLine;

void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
  Vex2D normal = { -(b->y - a->y), b->x - a->x };
  Vex2D new_normal = normal;
  
  x3d_normalize_vex2d_fp0x16(&new_normal);
  
  printf("{%d, %d} after {%d, %d}\n", normal.x, normal.y, new_normal.x, new_normal.y);
}

typedef struct X3D_BoundRegion {
  uint16 total_bl;
  X3D_BoundLine line[];
} X3D_BoundRegion;


void x3d_construct_boundregion(X3D_BoundRegion* region, Vex2D v[], uint16 total_v) {
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    x3d_construct_boundline(region->line + i, v + i, v + ((i + 1) % total_v));
  }
  
  region->total_bl = total_v;
}

typedef struct X3D_EdgeClip {
  uint16 v[2];
  uint16 clip_status[2];
} X3D_EdgeClip;

#define INDEX(_clip, _line, _v) _line * _clip->region->total_bl + _v

#define DIST(_clip, _line, _v) _clip->dist[INDEX(_clip, _line, _v)]

#define OUTSIDE(_clip, _v, _pos) _clip->dist[INDEX(_clip, _pos, _v)]

inline x3d_dist_to_line(X3D_BoundLine* line, Vex2D* v) {
  return (((int32)line->normal.x * v->x) + ((int32)line->normal.y * v->y)) >> X3D_NORMAL_SHIFT;
}


// inline x3d_calc_distances_to_lines(X3D_ClipData* clip) {
  // uint16 line, vex;
  
  // for(vex = 0; vex < clip->total_v; ++vex)
    // clip->outside_total[vex] = 0;
  
  // for(line = 0; line < clip->region->total_bl; ++line) {
    // for(vex = 0; vex < clip->total_v; ++vex) {
      // int16 dist = x3d_dist_to_line(clip->region->line + line, clip->prism->v + vex);
      
      // DIST(clip, line, vex) = dist;
      
      // if(dist < 0) {
        // OUTSIDE(clip, vex, clip->outside_total[line]++) = line;
      // }
    // }
  // }
// }

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

enum {
  CLIP_OUTSIDE,
  CLIP_INSIDE,
  CLIP_CLIPPED
};

// void x3d_clip_edge(X3D_ClipData* clip, uint16 id) {
  // uint16 a, b;
  
  // x3d_get_prism2d_edge(clip->prism, id, &a, &b);
  
  // uint16 a_pos = 0, b_pos = 0;
  
  // X3D_EdgeClip* edge = clip->seg_clip->edge + id;
  
  //Check to make sure that a and b don't fail by the same edge
  // while(a_pos < clip->outside_total[a] && b_pos < clip->outside_total[b]) {
    // uint16 out_a = OUTSIDE(clip, a, a_pos);
    // uint16 out_b = OUTSIDE(clip, b, b_pos);
    
    // if(out_a == out_b) {
      // edge->clip_status[0] = CLIP_OUTSIDE;
      // return;
    // }
    
    // if(out_a < out_b)
      // ++a_pos;
    // else
      // ++b_pos;
  // }
  
// }

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
  X3D_ClippedEdge*  edge_out;
  
  int16*            line_dist;
  uint16*           outside;
  uint16*           outside_total;

} X3D_ClipData;

static inline int16* dist_ptr(X3D_ClipData* clip, uint16 line, uint16 vertex) {
  return clip->line_dist + line * clip->region->total_bl + vertex;
}

static inline int16* outside_ptr(X3D_ClipData* clip, uint16 vertex, uint16 pos) {
  return clip->outside + vertex * clip->region->total_bl + pos;
}

static inline void add_outside(X3D_ClipData* clip, uint16 vertex, uint16 line) {
  *outside_ptr(clip, vertex, clip->outside_total[vertex]++) = line;
}

static inline int16 dist(X3D_ClipData* clip, uint16 line, uint16 vertex) {
  int16 n = dist(clip, line, vertex);
  
  
  
}

static inline calc_line_distances(X3D_ClipData* clip) {
  uint16 vertex, line;
  
  for(line = 0; line < clip->region->total_bl; ++line) {
    for(vertex = 0; vertex < clip->total_v; ++vertex) {  
      int16 dist = x3d_dist_to_line(clip->region->line + line, clip->v + vertex);     
      *dist_ptr(clip, line, vertex);
      
      if(dist < 0) {
        add_outside(clip, vertex, line);      
      }
    }
  }
}

//#define 

static inline clip_scale(X3D_ClipData* clip, uint16 start, uint16 end, uint16 line) {
  int16 n = -dist(clip, line, end);
  int16 d = abs(-dist(clip, line, end) + clip->region->line[line].d) + abs(-dist(clip, line, start) + clip->region->line[line].d);
  
  if(n == 0)
    return 0;
}

static inline clip_edge(X3D_ClipData* clip, X3D_IndexedEdge* edge, X3D_ClippedEdge* edge_out) {
}

void x3d_clip_edges(X3D_ClipData* clip) {
  // Allocate space
  clip->line_dist = alloca(sizeof(int16) * clip->total_v * clip->region->total_bl);
  clip->outside = alloca(sizeof(uint16) * clip->total_v * clip->region->total_bl);
  clip->outside_total = alloca(sizeof(uint16) * clip->total_v);
  
  uint16 i;
  for(i = 0; i < clip->total_v; ++i)
    clip->outside_total[i] = 0;
  
  // Calculate the distance from every point to every line
  calc_line_distances(clip);
  
  for(i = 0; i < clip->total_e; ++i)
    clip_edge(clip, clip->edge + i, clip->edge_out + i);
  
}
  
  
  
  
  // X3D_Prism2D* prism;
  // uint16 total_v = prism->base_v * 2;
  
  // int16 dist[region->total_bl * total_v];
  // uint16 outside[region->total_bl * total_v];
  // uint16 outside_total[total_v];
  
  
  // X3D_ClipData clip = {
    // .seg = seg,
    // .region = region,
    // .prism = prism,
    // .total_v = total_v,
    // .dist = dist,
    // .outside = outside,
    // .outside_total = outside_total
  // };
  
  
  // x3d_calc_distances_to_lines(&clip);
  
 

void x3d_test_new_clip() {
  X3D_BoundRegion* region = alloca(sizeof(X3D_BoundRegion) + sizeof(X3D_BoundLine) * 10);
  
  Vex2D v[] = {
    { 0, 0 },
    { LCD_WIDTH - 1, 0 },
    { LCD_WIDTH - 1, LCD_HEIGHT - 1 },
    { 0, LCD_HEIGHT - 1 }
  };
  
  x3d_construct_boundregion(region, v, 4);
}








































