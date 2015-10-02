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
#include "X3D_prism.h"
#include "X3D_memory.h"
#include "X3D_trig.h"
#include "X3D_segment.h"

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
  
#if 0
  Vex2D center = { LCD_WIDTH / 2, LCD_HEIGHT / 2 };
  
  if(x3d_dist_to_line(line, &center ) < 0) {
    line->normal.x = -line->normal.x;
    line->normal.y = -line->normal.y;
    line->d = -line->d;
  }
#endif
  
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

void fill_boundregion(X3D_BoundRegion* region) {
  uint16 i, d;
  uint16 line;
  
  for(i = 0; i < LCD_WIDTH; ++i) {
    for(d = 0; d < LCD_HEIGHT; ++d) {
      Vex2D v = { i, d};
      _Bool pass = TRUE;
      
      
      for(line = 0; line < region->total_bl; ++line) {
          if(x3d_dist_to_line(region->line + line, &v) < 0) {
            pass = FALSE;
            break;
          }
      }
      
      if(pass) {
        DrawPix(i, d, A_XOR);
      }
    }
  }
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
  CLIP_INVISIBLE,
  CLIP_VISIBLE,
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
  X3D_ClippedEdge*  edge_clip;
  
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
      
      if(d <= 0) {
        add_outside(clip, vertex, line);      
      }
    }
  }
  
  //printf("DONE\n");
}

//#define 

static inline int16 clip_scale(X3D_ClipData* clip, uint16 start, uint16 end, uint16 line) {
  int16 n = dist(clip, line, start);
  int16 d = abs(dist(clip, line, end)) + abs(dist(clip, line, start));
  
  if(n <= 0)
    return 0;
  
  if(n == d)
    return 0x7FFF;
  
  return ((int32)n << X3D_NORMAL_SHIFT) / d;
}

static inline int16 min_clip_scale(X3D_ClipData* clip, uint16 start, uint16 end, uint16* min_scale_line) {
  int16 min_scale = 0x7FFF;
  uint16 line = 0;
  
  for(line = 0; line < clip->outside_total[end]; ++line) {
    uint16 fail_line = *outside_ptr(clip, end, line);
    uint16 scale = clip_scale(clip, start, end, fail_line);
  
    if(scale == 0)
      return 0;
    
    if(scale < min_scale) {
      *min_scale_line = fail_line;
      min_scale = scale;
    }
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

static inline void clip_edge(X3D_ClipData* clip, X3D_IndexedEdge* edge, X3D_ClippedEdge* edge_out) {
  uint16 vex;
  for(vex = 0; vex < 2; ++vex) {
    
    if(!vertex_visible(clip, edge->v[vex])) {
      uint16 min_scale_line;
      int16 scale = min_clip_scale(clip, edge->v[vex ^ 1], edge->v[vex], &min_scale_line);
      
      //printf("Scale: %d\n", scale);
      
      if(scale != 0) {
        scale_edge(&edge_out->v[vex].v, &clip->v[edge->v[vex ^ 1]], &clip->v[edge->v[vex]], scale);
        edge_out->v[vex].clip_line = min_scale_line;
        edge_out->v[vex].clip_status = CLIP_CLIPPED;
      }
      else {
        edge_out->v[0].clip_status = CLIP_INVISIBLE;
        edge_out->v[1].clip_status = CLIP_INVISIBLE;
        return;
      }
    }
    else {
      edge_out->v[vex].clip_status = CLIP_VISIBLE;
      edge_out->v[vex].v = clip->v[edge->v[vex]];
    }
  }
  
  if(!vertex_visible(clip, edge->v[0]) && !vertex_visible(clip, edge->v[1])) {
    // Check to make sure the midpoint is in the bounding region to disambiguate
    // the cases where an egde fails by two separate bounding lines, and interesecs
    // the bounding region and when it doesn't.
    Vex2D mid = { (edge_out->v[0].v.x + edge_out->v[1].v.x) / 2, (edge_out->v[0].v.y + edge_out->v[1].v.y) / 2 };
    uint16 line;
    
    for(line = 0; line < clip->region->total_bl; ++line) {
      if(x3d_dist_to_line(clip->region->line + line, &mid) < 0) {
        edge_out->v[0].clip_status = CLIP_INVISIBLE;
        edge_out->v[1].clip_status = CLIP_INVISIBLE;
        return;
      }
    }
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
    clip_edge(clip, clip->edge + i, clip->edge_clip + i);
  
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

Vex2D rand_side(uint16 s) {
  Vex2D v;
  
  if(s == 0 || s == 1) {
    v.x = (s == 0 ? 0 : LCD_WIDTH - 1);
    v.y = rand() % LCD_HEIGHT;
  }
  else {
    v.x = rand() % LCD_WIDTH;
    v.y = (s == 2 ? 0 : LCD_HEIGHT - 1);
  }
  
  return v;
}

void rand_sides(uint16* a, uint16* b) {
  *a = rand() % 4;
  
  do {
    *b = rand() % 4;
  } while(*b == *a);
}

void rand_line(Vex2D* start, Vex2D* end) {
  uint16 a, b;
  
  rand_sides(&a, &b);
  *start = rand_side(a);
  *end = rand_side(b);
}

void draw_line(Vex2D a, Vex2D b) {
  DrawLine(a.x, a.y, b.x, b.y, A_NORMAL);
}

void TEST_x3d_project_prism3d(X3D_Prism2D* dest, X3D_Prism3D* p, X3D_ViewPort* context);

void generate_prism2d_edge_list(X3D_Prism2D* prism, X3D_IndexedEdge* edges) {
  uint16 i;
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    x3d_get_prism2d_edge(prism, i, edges[i].v, edges[i].v + 1);
  }
}

#define EDGE(_id) clip->edge_clip[edge_list[_id]]

void x3d_construct_boundregion_from_clip_data(X3D_ClipData* clip, uint16* edge_list, uint16 total_e, X3D_BoundRegion* region) {
  region->total_bl = 0;
  
  uint16 edge_id = 0;
  
  // Skip over edges that are totally invisible
  while(edge_id < total_e && EDGE(edge_id).v[0].clip_status == CLIP_INVISIBLE) {
    ++edge_id;
  }
  
  uint16 first_visible_edge = edge_id;
  
  if(edge_id < total_e) {
    
    do {
      // Alright, so we've encountered an edge that is at least partially visible
      X3D_ClippedEdge* edge = &EDGE(edge_id);
      
      // We're only interested in edges that are either totally visible, or begin in the
      // bounding region and exit
      if(edge->v[0].clip_status == CLIP_VISIBLE) {
        if(edge->v[1].clip_status == CLIP_CLIPPED) {
          
          
          // Construct a bounding line for the edge
          x3d_construct_boundline(region->line + region->total_bl++, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v);
          
          
          uint16 start_edge = edge_id;
          
          // Walk along the old bounding region until we find an edge where we reenter it
          do {
            edge_id = x3d_single_wrap(edge_id + 1, total_e);
          } while(EDGE(edge_id).v[1].clip_status != CLIP_VISIBLE);
          
          // Add the edges along the old bound region
          uint16 start = EDGE(start_edge).v[1].clip_line;
          uint16 end = EDGE(edge_id).v[0].clip_line;
          
          //printf("\n\n\n\n\n\n\n\n\n\n\n\nStart: %d, %d\nId: %d, %d\n", start, end, start_edge, edge_id);
          
          region->line[region->total_bl++] = clip->region->line[start];
          
          
          // Prevent special case of when it enters and exits on the same edge
          if(start != end) {
            uint16 e = start;
            
            do {
              e = x3d_single_wrap(e + 1, clip->region->total_bl);
              region->line[region->total_bl++] = clip->region->line[e];
            } while(e != end);
          }
          
        }
        
        // TODO construct a new bounding line for the next clipped edge
        x3d_construct_boundline(region->line + region->total_bl++, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v);
      }
      
      edge_id = x3d_single_wrap(edge_id + 1, total_e);    
    }
    while(edge_id != first_visible_edge);
    
    
  }
  else {
    printf("Invisible\n");
  }
  
  
}

void test_clip_scale(X3D_Context* context, X3D_ViewPort* port) {
  srand(0);
  
  uint16 i, d;
  
  X3D_BoundRegion* region = alloca(sizeof(X3D_BoundLine) * 4 + sizeof(X3D_BoundRegion));
  
  X3D_ClipData clip = {
    .region = region,
    .total_v = 2,
    .total_e = 0
  };
  
  clip.line_dist = alloca(400);
  clip.outside = alloca(400);
  clip.outside_total = alloca(400);
  clip.edge_clip = alloca(400);
  clip.edge = alloca(400);

  
  Vex2D p[] = {
    { 40, 20 },
    { 200, 70 + 15 + 5 },
    { 150, 100 },
    { 20, 40 }
  };
  
  x3d_construct_boundregion(region, p, 4);
  
  for(i = 0; i < 20; ++i)
    clip.outside_total[i] = 0;
  
  
  DrawPix(LCD_WIDTH / 2, LCD_HEIGHT / 2, A_NORMAL);
  
  
  X3D_Prism3D* prism = alloca(500);
  
  X3D_Mat3x3_fp0x16 mat;
  Vex3D_angle256 angle = { 0, 0, 0 };
  x3d_mat3x3_fp0x16_construct(&mat, &angle);
  
  x3d_prism_construct(prism, 8, 50, 100, (Vex3D_angle256) { ANG_45, 0, 0 });
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    prism->v[i].z += 400;
  }
  
  
  X3D_Prism2D* prism2d = alloca(500);
  
  TEST_x3d_project_prism3d(prism2d, prism, port);
  
  for(i = 0; i < prism2d->base_v * 2; ++i) {
    prism2d->v[i].y += 27;
  }
  
  
  clrscr();
  for(i = 0; i < 4; ++i) {
    uint16 next = (i + 1) % 4;
    draw_line(p[i], p[next]);
  }

  //fill_boundregion(region);
  
  clip.total_e = prism2d->base_v * 3;
  
  for(i = 0; i < prism2d->base_v * 3; ++i) {
    uint16 a, b;
    
    x3d_get_prism2d_edge(prism2d, i, &a, &b);
    
    draw_line(prism2d->v[a], prism2d->v[b]);
  }
  
  
  LCD_restore(context->screen_data);
  
  while(!_keytest(RR_Q)) ;
  
  clrscr();
  
  clip.total_v = prism2d->base_v * 2;
  clip.v = prism2d->v;
  
  calc_line_distances(&clip);
  
#if 1
  for(i = 0; i < 4; ++i) {
    uint16 next = (i + 1) % 4;
    draw_line(p[i], p[next]);
  }

  

  generate_prism2d_edge_list(prism2d, clip.edge);
#if 1
  x3d_clip_edges(&clip);
  
  
  for(i = 0; i < prism2d->base_v * 3; ++i) {
    
    if(clip.edge_clip[i].v[0].clip_status != CLIP_INVISIBLE) {
      draw_line(clip.edge_clip[i].v[0].v, clip.edge_clip[i].v[1].v);
    }
  }
#endif
#endif
  
  uint16 edges[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  
  X3D_BoundRegion* new_region = alloca(sizeof(X3D_BoundLine) * 4 + sizeof(X3D_BoundRegion));
  
  x3d_construct_boundregion_from_clip_data(&clip, edges, 8, new_region);
  
  fill_boundregion(new_region);
  
  printf("Bl: %d\n", new_region->total_bl);
  
  printf("clipped\n");
  
  LCD_restore(context->screen_data);
  
  while(1) ;
  
  /*
  randomize();
  
  Vex2D start, end;
  rand_line(&start, &end);
  
  //draw_line(start, end);
  
  // clip_edge(X3D_ClipData* clip, X3D_IndexedEdge* edge, X3D_ClippedEdge* edge_out)
  
  Vex2D points[] = {
    { -8192, -8192},
    { 8192, 8192 }
  };
  
  clip.v = points;
  
  calc_line_distances(&clip);
  
  X3D_IndexedEdge edge = {
    {0, 1} 
  };
  
  X3D_ClippedEdge edge_out;
  
  
  
  
  
  
  
  
  clip_edge(&clip, &edge, &edge_out);
  
  printf("start stat: %u, end stat: %u\n", edge_out.v[0].clip_status, edge_out.v[1].clip_status);
  
  ngetchx();
  
  clrscr();
  for(i = 0; i < 4; ++i) {
    uint16 next = (i + 1) % 4;
    draw_line(p[i], p[next]);
  }
  
  if(edge_out.v[0].clip_status != CLIP_INVISIBLE)
    draw_line(edge_out.v[0].v, edge_out.v[1].v);
  
  
  Vex2D s = { 0, 0 };
  Vex2D e = { 127, 127 };
  
  */
  
  
  //draw_line(s, e);
  
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
  
 

void x3d_test_new_clip(X3D_Context* context, X3D_ViewPort* port) {
  test_clip_scale(context, port);
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








































