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

inline int16 x3d_dist_to_line(X3D_BoundLine* line, Vex2D* v) {
  return ((((int32)line->normal.x * v->x) + ((int32)line->normal.y * v->y)) >> X3D_NORMAL_SHIFT) + line->d;
}

void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
  Vex2D normal = { -(b->y - a->y), b->x - a->x };
  Vex2D new_normal = normal;
  
  x3d_normalize_vex2d_fp0x16(&new_normal);
  
  line->normal = new_normal;
  line->d = (((int32)-line->normal.x * a->x) - ((int32)line->normal.y * a->y)) >> X3D_NORMAL_SHIFT;
  
  Vex2D center = { LCD_WIDTH / 2, LCD_HEIGHT / 2 };
  
  if(x3d_dist_to_line(line, &center ) < 0) {
    line->normal.x = -line->normal.x;
    line->normal.y = -line->normal.y;
    line->d = -line->d;
  }
  
  //printf("{%d, %d} after {%d, %d}, %d\n", normal.x, normal.y, new_normal.x, new_normal.y, line->d);
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
  return clip->line_dist + line * clip->total_v + vertex;
}

static inline int16* outside_ptr(X3D_ClipData* clip, uint16 vertex, uint16 pos) {
  return clip->outside + vertex * clip->region->total_bl + pos;
}

static inline void add_outside(X3D_ClipData* clip, uint16 vertex, uint16 line) {
  *outside_ptr(clip, vertex, clip->outside_total[vertex]++) = line;
}

static inline int16 dist(X3D_ClipData* clip, uint16 line, uint16 vertex) {
  int16 n = *dist_ptr(clip, line, vertex);
  
  return n;
  
}

static inline calc_line_distances(X3D_ClipData* clip) {
  uint16 vertex, line;
  
  for(line = 0; line < clip->region->total_bl; ++line) {
    for(vertex = 0; vertex < clip->total_v; ++vertex) {  
      int16 d = x3d_dist_to_line(clip->region->line + line, clip->v + vertex);     
      *dist_ptr(clip, line, vertex) = d;
      
      //printf("dist: %d (offset %d) dist: %d\n", d, (int16)(dist_ptr(clip, line, vertex) - clip->line_dist), dist(clip, line, vertex));
      
      if(d < 0) {
        add_outside(clip, vertex, line);      
      }
    }
  }
  
  //printf("DONE\n");
}

//#define 

static inline int16 clip_scale(X3D_ClipData* clip, uint16 start, uint16 end, uint16 line) {
  int16 n = abs(dist(clip, line, start));
  int16 d = abs(dist(clip, line, end)) + abs(dist(clip, line, start));
  
  if(n == 0)
    return 0;
  
  if(n == d)
    return 0x7FFF;
  
  return ((int32)n << X3D_NORMAL_SHIFT) / d;
}

static inline int16 min_clip_scale(X3D_ClipData* clip, uint16 start, uint16 end) {
  int16 min_scale = 0x7FFF;
  uint16 line = 0;
  
  for(line = 0; line < clip->outside_total[end]; ++line) {
    uint16 scale = clip_scale(clip, start, end, *outside_ptr(clip, end, line));
  
    if(scale == 0)
      return 0;
    
    min_scale = min(min_scale, scale);
  }
  
  return min_scale;
}

void scale_edge(Vex2D* end_dest, Vex2D* start, Vex2D* end, int16 scale) {
  int16 dx = end->x - start->x;
  int16 dy = end->y - start->y;
  
  end_dest->x = start->x + (((int32)dx * scale) >> X3D_NORMAL_SHIFT);
  end_dest->y = start->y + (((int32)dy * scale) >> X3D_NORMAL_SHIFT);
}

int16 clip_scale_float(X3D_ClipData* clip, uint16 start, uint16 end, uint16 line) {
  float n = abs(dist(clip, line, start));
  float d = abs(dist(clip, line, end)) + abs(dist(clip, line, start));
  
  return (n / d) * 32768.0;
}

static inline _Bool vertex_visible(X3D_ClipData* clip, uint16 v) {
  return clip->outside_total[v] == 0;
}

static inline clip_edge(X3D_ClipData* clip, X3D_IndexedEdge* edge, X3D_ClippedEdge* edge_out) {
  // TODO: make sure the endpoints don't fail by the same line
  
  _Bool out_a = vertex_visible(clip, edge->v[0]);
  _Bool out_b = vertex_visible(clip, edge->v[1]);
  
  edge_out->v[0] = clip->v[edge->v[0]];
  edge_out->v[1] = clip->v[edge->v[1]];
  
  if(out_a) {
    
  }
}

int16 ask(const char* str) {
  printf(str);
  
  char buf[32];
  gets(buf);
  printf("\n");
  
  return atoi(str);
}

Vex2D ask_vex2d(const char* str) {
  printf("%d:\n", str);
  
  Vex2D v;
  v.x = ask("x: ");
  v.x = ask("y: ");
  
  return v;
}

Vex2D rand_vex2d() {
  return (Vex2D) { rand() % LCD_WIDTH, rand() % LCD_HEIGHT };
  //return (Vex2D) { rand() - 16384, rand() - 16384 };
}

void test_clip_scale() {
  srand(0);
  
  uint16 i, d;
  
  X3D_BoundRegion* region = alloca(sizeof(X3D_BoundLine) * 4 + sizeof(X3D_BoundRegion));
  
  X3D_ClipData clip = {
    .region = region,
    .total_v = 2,
    .total_e = 0
  };
  
  clip.line_dist = alloca(100);
  clip.outside = alloca(100);
  clip.outside_total = alloca(100);
  
  for(i = 0; i < 1; ++i) {
    Vex2D b1;// = rand_vex2d();
    Vex2D b2;// = rand_vex2d();
    
    for(d = 0; d < 10; ++d)
      clip.outside_total[d] = 0;
    
    
    b1.x = rand() % LCD_WIDTH;
    b1.y = (rand() % 2) * (LCD_HEIGHT - 1);
    
    b2.x = (rand() % 2) * (LCD_WIDTH - 1);
    b2.y = rand() % LCD_HEIGHT;
    
    X3D_BoundLine line;
    
    x3d_construct_boundline(&line, &b1, &b2);
    
    Vex2D p1, p2;
    int16 d1, d2;
    
    
    do {
      p1 = rand_vex2d();
      p2 = rand_vex2d();
      
      d1 = x3d_dist_to_line(&line, &p1);
      d2 = x3d_dist_to_line(&line, &p2);
      
      //printf("distances: %d %d", d1, d2);
    } while(d1 < 0 || d2 >= 0);
    
    //printf("VEX: %d %d, %d %d\n", p1.x, p1.y, p2.x, p2.y);
    
    Vex2D v[] = { p1, p2 };
    
    clip.v = v;
 
    region->total_bl = 1;
    region->line[0] = line;
    

    
    
     
    calc_line_distances(&clip);
    
    int16 scale = clip_scale(&clip, 0, 1, 0);
    int16 min_s = min_clip_scale(&clip, 0, 1);
    
    //printf("CLIP\n");
    
    int16 scale_float = clip_scale_float(&clip, 0, 1, 0);
    
    if(scale != scale_float) {
      //printf("Scale: %d, scale_float: %d\n", scale, scale_float);
    }
    
    DrawLine(b1.x, b1.y, b2.x, b2.y, A_NORMAL);
    DrawLine(p1.x, p1.y, p2.x, p2.y, A_NORMAL);
    printf("Scale: %d, scale_float: %d, min: %d\n", scale, scale_float, min_s);
    ngetchx();
    
    Vex2D new_end;
    scale_edge(&new_end, &p1, &p2, scale);
    
    clrscr();
    DrawLine(b1.x, b1.y, b2.x, b2.y, A_NORMAL);
    DrawLine(p1.x, p1.y, new_end.x, new_end.y, A_NORMAL);
    printf("Scale: %d, scale_float: %d\n", scale, scale_float);
    ngetchx();
    clrscr();

    //ngetchx();
  }
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
  test_clip_scale();
  return;
  
  
  X3D_BoundRegion* region = alloca(sizeof(X3D_BoundRegion) + sizeof(X3D_BoundLine) * 10);
  
  Vex2D v[] = {
    { 0, 0 },
    { LCD_WIDTH - 1, 0 },
    { LCD_WIDTH - 1, LCD_HEIGHT - 1 },
    { 0, LCD_HEIGHT - 1 }
  };
  
  Vex2D vex[] = { {0, 0} };
  
  X3D_ClipData clip = {
    .region = region,
    .v = vex,
    .total_v = 1,
    .total_e = 0
  }; 
  
  x3d_construct_boundregion(region, v, 4);
  x3d_clip_edges(&clip);
}








































