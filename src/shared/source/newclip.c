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

_Bool swap_boundline;

void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b, _Bool clockwise) {
  #if 0
  if(!clockwise) {
    Vex2D* temp = a;
    a = b;
    b = temp;
}
#endif

Vex2D normal = { -(b->y - a->y), b->x - a->x };
Vex2D new_normal = normal;

x3d_normalize_vex2d_fp0x16(&new_normal);

line->normal = new_normal;
line->d = (((int32)-line->normal.x * a->x) - ((int32)line->normal.y * a->y)) >> X3D_NORMAL_SHIFT;

line->normal.x = -line->normal.x;
line->normal.y = -line->normal.y;
line->d = -line->d;

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
  Vex2D point_inside;
  X3D_BoundLine line[];
} X3D_BoundRegion;


uint16 get_prism2d_face_edges(X3D_Prism2D* prism, uint16 face, uint16* edges) {
  uint16 i;
  
  if(face == BASE_A) {
    for(i = 0; i < prism->base_v; ++i) {
      edges[i] = i;
    }
    
    return prism->base_v;
  }
  else if(face == BASE_B) {
    for(i = 0; i < prism->base_v; ++i) {
      edges[i] = prism->base_v * 2 - i - 1;
    }
    
    return prism->base_v;
  }
}

_Bool is_clockwise_turn(Vex2D* p1, Vex2D* p2, Vex2D* p3) {
  Vex2D a = { p2->x - p1->x, p2->y - p1->y };
  Vex2D b = { p3->x - p2->x, p3->y - p2->y };
  
  return (int32)a.x * b.y - (int32)b.x * a.y < 0;
}



void x3d_construct_boundregion(X3D_BoundRegion* region, Vex2D v[], uint16 total_v) {
  int16 i;
  
  if(is_clockwise_turn(v, v + 1, v + 2)) {
    
    for(i = 0; i < total_v; ++i) {
      x3d_construct_boundline(region->line + i, v + i, v + ((i + 1) % total_v), 1);
    }
  }
  else {
    for(i = total_v - 1; i >= 0; --i) {
      x3d_construct_boundline(region->line + i, v + i, (i != 0 ? v + i - 1 : v + total_v - 1), 1);
    }
  }
  
  region->point_inside = v[0];
  
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
        DrawPix(i, d, A_NORMAL);
      }
      else {
        DrawPix(i, d, A_REVERSE);
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

// TODO: fix order on BASE_B
inline void x3d_get_prism2d_edge(X3D_Prism2D* p, uint16 id, uint16* a, uint16* b) {
  if(id < p->base_v) {
    *a = id;
    
    if(id != p->base_v - 1)
      *b = id + 1;
    else
      *b = 0;
  }
  else if(id < p->base_v * 2) {
    if(id != p->base_v * 2 - 1)
      *a = x3d_prism2d_opposite_vertex(p, id + 1 - p->base_v);
    else
      *a = x3d_prism2d_opposite_vertex(p, p->base_v - p->base_v);
    
    *b = x3d_prism2d_opposite_vertex(p, id - p->base_v);
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
  uint32*           outside_mask;
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
  
  for(vertex = 0; vertex < clip->total_v; ++vertex) {
    uint32 outside_mask = 0;
    
    for(line = 0; line < clip->region->total_bl; ++line) {  
      
      int16 d = x3d_dist_to_line(clip->region->line + line, clip->v + vertex);     
      *dist_ptr(clip, line, vertex) = d;
      
      //printf("dist: %d (offset %d) dist: %d\n", d, (int16)(dist_ptr(clip, line, vertex) - clip->line_dist), dist(clip, line, vertex));
      
      outside_mask = (outside_mask << 1) | (d <= 0 ? 1 : 0);
      
      if(d <= 0) {
        add_outside(clip, vertex, line);      
      }
    }
    
    clip->outside_mask[vertex] = outside_mask;
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

#define SIGNOF(_x) ((_x) == 0 ? 0 : ((_x) < 0) ? -1 : 1) 

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
    // Check to make sure the domain wasn't flipped
    int16 old_dx = clip->v[edge->v[0]].x - clip->v[edge->v[1]].x;
    int16 old_dy = clip->v[edge->v[0]].y - clip->v[edge->v[1]].y;
    
    int16 new_dx = edge_out->v[0].v.x - edge_out->v[1].v.x;
    int16 new_dy = edge_out->v[0].v.y - edge_out->v[1].v.y;
    
    if(SIGNOF(new_dx) != SIGNOF(old_dx) || SIGNOF(new_dy) != SIGNOF(old_dy)) {
      edge_out->v[0].clip_status = CLIP_INVISIBLE;
      edge_out->v[1].clip_status = CLIP_INVISIBLE;
      return;
    }
  }
  
  #if 0
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
        //printf("CASE\n");
        
        printf("FAIL FAIL FAIL\n");
        while(1) ;
        
        return;
}
}
}
#endif
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


#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };

_Bool diff_boundline(X3D_BoundLine* a, X3D_BoundLine* b) {
  return TRUE;//abs(a->normal.x - b->normal.x) > 500 || abs(a->normal.y - b->normal.y) > 500 || abs(a->d - b->d) > 2;
}

X3D_BoundRegion* x3d_construct_boundregion_from_clip_data(X3D_ClipData* clip, uint16* edge_list, uint16 total_e, X3D_BoundRegion* region, _Bool clockwise) {
  X3D_BoundRegion* result_region = region;
  
  region->total_bl = 0;
  
  int16 reverse_edge_list[total_e];
  
  // If not clockwise, reverse the list of edges
  if(!clockwise) {
    uint16 i;
    
    printf("NOT Clockwise\n");
    
    for(i = 0; i < total_e; ++i) {
      reverse_edge_list[i] = edge_list[total_e - i - 1];
      
      SWAP(EDGE(i).v[0], EDGE(i).v[1]);
    }
    
    edge_list = reverse_edge_list;
    
  }
  
  uint16 a;
  for(a = 0; a < total_e; ++a) {
    printf("EDGE %d{%d} -> %d{%d}: %d, %d\n", EDGE(a).v[0].clip_line, EDGE(a).v[0].clip_status, EDGE(a).v[1].clip_line, EDGE(a).v[1].clip_status, EDGE(a).v[0].v.x, EDGE(a).v[0].v.y);
  }
  
  
  uint16 edge_id = 0;
  
  // Skip over edges that are totally invisible
  while(edge_id < total_e && EDGE(edge_id).v[0].clip_status == CLIP_INVISIBLE) {
    ++edge_id;
  }
  
  uint16 first_visible_edge = 0xFFFF;
  
  if(edge_id < total_e) {
    
    while(edge_id != first_visible_edge) {
      // Alright, so we've encountered an edge that is at least partially visible
      X3D_ClippedEdge* edge = &EDGE(edge_id);
      
      // We're only interested in edges that are either totally visible, or begin in the
      // bounding region and exit
      if(edge->v[0].clip_status == CLIP_VISIBLE || edge->v[0].clip_status == CLIP_CLIPPED) {
        if(first_visible_edge == 0xFFFF) {
          first_visible_edge = edge_id;
          region->point_inside = edge->v[0].v;
        }
        
        if(edge->v[1].clip_status == CLIP_CLIPPED) {
          
          // Construct a bounding line for the edge
          x3d_construct_boundline(region->line + region->total_bl, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v, clockwise);
          
          if(region->total_bl == 0 || diff_boundline(region->line + region->total_bl, region->line + region->total_bl - 1))
            ++region->total_bl;
          
          
          uint16 start_edge = edge_id;
          
          // Walk along the old bounding region until we find an edge where we reenter it
          do {
            edge_id = x3d_single_wrap(edge_id + 1, total_e);
          } while(EDGE(edge_id).v[1].clip_status != CLIP_VISIBLE && EDGE(edge_id).v[1].clip_status != CLIP_CLIPPED);
          
          // Add the edges along the old bound region
          uint16 start = EDGE(start_edge).v[1].clip_line;
          uint16 end = EDGE(edge_id).v[0].clip_line;
          
          if(diff_boundline(clip->region->line + start, region->line + region->total_bl - 1))
            region->line[region->total_bl++] = clip->region->line[start];
          
          printf("Start: %d, end: %d\n", start, end);
          
          // Prevent special case of when it enters and exits on the same edge
          if(start != end) {
            uint16 e = start;
            
            do {
              e = x3d_single_wrap(e + 1, clip->region->total_bl);
              
              if(diff_boundline(clip->region->line + e, region->line + region->total_bl - 1))
                region->line[region->total_bl++] = clip->region->line[e];
            } while(e != end);
          }
          else {
            
          }
          
          continue;
          
        }
        
        
        x3d_construct_boundline(region->line + region->total_bl, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v, clockwise);
        
        if(region->total_bl == 0 || diff_boundline(region->line + region->total_bl, region->line + region->total_bl - 1))
          ++region->total_bl;
      }
      
      edge_id = x3d_single_wrap(edge_id + 1, total_e);
    }
    
    
  }
  else {
    uint16 i;
    uint32 edge_mask = (1L << clip->region->total_bl) - 1;// = clip->outside_mask[clip->edge->v[0]];
    
    /*for(i = 0; i < total_e; ++i) {
     *      edge_mask |= i;//(1L << (clip->region->total_bl - edge_list[i] - 1));
     *      printf("EDge: %d\n", edge_list[i]);
  }*/
    
    printf("Begin: %ld\n", edge_mask);
    
    for(i = 0; i < total_e; ++i) {
      printf("Mask: %ld\n", clip->outside_mask[clip->edge[edge_list[i]].v[0]]);
      edge_mask &= clip->outside_mask[clip->edge[edge_list[i]].v[0]];
    }
    
    if(edge_mask != 0) {
      result_region = NULL;
      printf("EDGE FAIL\n");
    }
    else {
      for(i = 0; i < total_e; ++i) {
        if(is_clockwise_turn(&EDGE(i).v[0].v, &clip->region->point_inside, &EDGE(i).v[1].v)) {
          result_region = NULL;
          break;
        }
      }
      
      #if 0
      X3D_BoundLine line;
      
      for(i = 0; i < total_e; ++i) {
        x3d_construct_boundline(&line, &EDGE(i).v[0].v, &EDGE(i).v[1].v, 0);
        
        if(x3d_dist_to_line(&line, &clip->region->point_inside) < 0) {
          if(result_region != NULL) {
            printf("POINT IN POLYGON FAILED\n");
    }
    
    result_region = NULL;
    break;
    }
    }
    #endif
    
    // If we got through testing all the edges, and the point is inside, the
    // old region must be inside the new region
    if(result_region != NULL) {
      result_region = clip->region;
      printf("ASSIGN OLD\n");
    }
    }
  }
  
  // Swap them back
  if(!clockwise) {
    uint16 i;
    
    for(i = 0; i < total_e; ++i) {
      SWAP(EDGE(i).v[0], EDGE(i).v[1]);
    }
  }
  
  return result_region;
}

X3D_BoundRegion* x3d_construct_boundregion_from_prism2d_face(X3D_ClipData* clip, X3D_Prism2D* prism, uint16 face, X3D_BoundRegion* region) {
  uint16 edges[prism->base_v];
  
  uint16 total_e = get_prism2d_face_edges(prism, face, edges);
  
  uint16 v[3] = {
    clip->edge[edges[0]].v[0],
    clip->edge[edges[0]].v[1],
    clip->edge[edges[1]].v[0]
  };
  
  _Bool clockwise = is_clockwise_turn(clip->v + v[0], clip->v + v[1], clip->v + v[2]);
  
  printf("Clockwise: %d\n", clockwise);
  
  return x3d_construct_boundregion_from_clip_data(clip, edges, total_e, region, clockwise);
}

uint16 input_polygon2d(Vex2D* v) {
  uint16 total_v = 0;
  uint16 cursor_x = LCD_WIDTH / 2, cursor_y = LCD_HEIGHT / 2;
  
  uint16 old_x = cursor_x, old_y = cursor_y;
  uint16 old_value = GetPix(old_x, old_y);
  
  _Bool start = 1;
  
  do {
    if(cursor_x != old_x || cursor_y != old_y || start) {
      DrawPix(old_x, old_y, old_value ? A_NORMAL : A_REVERSE);
      old_value = GetPix(cursor_x, cursor_y);
      old_x = cursor_x;
      old_y = cursor_y;
      DrawPix(cursor_x, cursor_y, A_NORMAL);
      start = 0;
      
      idle();
      
      if(_keytest(RR_DIAMOND)) {
        idle();
        idle();
        idle();
        idle();
      }
    }
    
    
    if(_keytest(RR_W)) {
      if(cursor_y > 0)
        --cursor_y;
    }
    else if(_keytest(RR_S)) {
      if(cursor_y + 1  < LCD_HEIGHT)
        ++cursor_y;
    }
    else if(_keytest(RR_A)) {
      if(cursor_x > 0)
        --cursor_x;
    }
    if(_keytest(RR_D)) {
      if(cursor_x + 1 < LCD_WIDTH)
        ++cursor_x;
    }
    else if(_keytest(RR_ENTER)) {
      v[total_v++] = (Vex2D) { cursor_x, cursor_y };
      
      if(total_v > 1)
        draw_line(v[total_v - 2], v[total_v - 1]  );
      
      while(_keytest(RR_ENTER)) ;
    }
    
    else if(_keytest(RR_ESC)) {
      while(_keytest(RR_ESC)) ;
      
      draw_line(v[0], v[total_v - 1]);
      return total_v;
    }
    
  } while(1);
  
  
  
}

void test_clip_scale(X3D_Context* context, X3D_ViewPort* port) {
  srand(0);
  
  Vex2D clip_v[20];
  
  //clrscr();
  
  
  uint16 i, d;
  
  X3D_BoundRegion* region = alloca(sizeof(X3D_BoundLine) * 20 + sizeof(X3D_BoundRegion));
  
  X3D_ClipData clip = {
    .region = region,
    .total_v = 2,
    .total_e = 0
  };
  
  clip.line_dist = malloc(400);
  clip.outside = malloc(400);
  clip.outside_total = malloc(400);
  clip.edge_clip = malloc(400);
  clip.edge = malloc(400);
  clip.outside_mask = malloc(400);
  
  Vex2D* p = clip_v;
  
  for(i = 0; i < 20; ++i)
    clip.outside_total[i] = 0;
  
  X3D_Prism3D* prism = alloca(500);  
  X3D_Mat3x3_fp0x16 mat;
  Vex3D_angle256 angle = { 0, 0, 0 };
  x3d_mat3x3_fp0x16_construct(&mat, &angle);
  
  x3d_prism_construct(prism, 4, 180, 50, (Vex3D_angle256) { ANG_180, ANG_45, 0 });
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    prism->v[i].z += 400;
  }
  
  
  X3D_Prism2D* prism2d = alloca(500);
  
  TEST_x3d_project_prism3d(prism2d, prism, port);
  
  for(i = 0; i < prism2d->base_v * 2; ++i) {
    prism2d->v[i].x += 20;
  }
  
  
  clrscr();
  clip.total_e = prism2d->base_v * 3;
  
  for(i = 0; i < prism2d->base_v * 3; ++i) {
    uint16 a, b;
    
    x3d_get_prism2d_edge(prism2d, i, &a, &b);
    
    draw_line(prism2d->v[a], prism2d->v[b]);
  }
  
  uint16 total_clip_v = input_polygon2d(clip_v);
  x3d_construct_boundregion(region, p, total_clip_v);
  
  for(i = 0; i < total_clip_v; ++i) {
    uint16 next = (i + 1) % total_clip_v;
    draw_line(p[i], p[next]);
  }
  
  while(!_keytest(RR_Q)) ;
  
  clrscr();
  
  clip.total_v = prism2d->base_v * 2;
  clip.v = prism2d->v;
  
  calc_line_distances(&clip);
  
  for(i = 0; i < total_clip_v; ++i) {
    uint16 next = (i + 1) % total_clip_v;
    draw_line(p[i], p[next]);
  }
  
  
  
  generate_prism2d_edge_list(prism2d, clip.edge);
  x3d_clip_edges(&clip);
  
  
  for(i = 0; i < prism2d->base_v * 3; ++i) {
    
    if(clip.edge_clip[i].v[0].clip_status != CLIP_INVISIBLE) {
      draw_line(clip.edge_clip[i].v[0].v, clip.edge_clip[i].v[1].v);
    }
  }
  
  uint16 edges[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  
  X3D_BoundRegion* new_region = alloca(sizeof(X3D_BoundLine) * 12 + sizeof(X3D_BoundRegion));
  
  uint16 new_edges[20];
  get_prism2d_face_edges(prism2d, BASE_B, new_edges);
  
  //x3d_construct_boundregion_from_clip_data(&clip, new_edges, 8, new_region, FALSE);
  
  new_region = x3d_construct_boundregion_from_prism2d_face(&clip, prism2d, BASE_B, new_region);
  
  if(new_region == NULL) {
    printf("INVISIBLE\n");
    
    while(1) ;
  }
  
  printf("Bl: %d\n", new_region->total_bl);
  
  while(!_keytest(RR_W)) ;
  
  uint16 bl = new_region->total_bl;
  
  clrscr();
  
  void* image[30];
  
  for(i = 0; i <= bl; ++i) {
    new_region->total_bl = i;
    
    uint16 d;
    
    for(d = 0; d < prism2d->base_v * 3; ++d) {
      uint16 a, b;
      
      x3d_get_prism2d_edge(prism2d, d, &a, &b);
      
      draw_line(prism2d->v[a], prism2d->v[b]);
    }
    
    for(d = 0; d < total_clip_v; ++d) {
      uint16 next = (d + 1) % total_clip_v;
      draw_line(p[d], p[next]);
    }
    
    fill_boundregion(new_region);
    image[i] = malloc(LCD_SIZE);
    memcpy(image[i], LCD_MEM, LCD_SIZE);
    //printf("{%d, %d} -> %d\n", new_region->line[i].normal.x, new_region->line[i].normal.y, new_region->line[i].d);
  }
  
  do {
    clrscr();
    memcpy(LCD_MEM, image[bl], LCD_SIZE);
    
    for(i = 0; i < prism2d->base_v * 3; ++i) {
      uint16 a, b;
      
      x3d_get_prism2d_edge(prism2d, i, &a, &b);
      
      draw_line(prism2d->v[a], prism2d->v[b]);
    }
    
    for(i = 0; i < total_clip_v; ++i) {
      uint16 next = (i + 1) % total_clip_v;
      draw_line(p[i], p[next]);
    }
    
    printf("line = %d\nTotal: %d\n", bl, new_region->total_bl);
    
    for(i = 0; i < new_region->total_bl; ++i) {
      printf("LINE: {%d %d} %d\n", new_region->line[i].normal.x, new_region->line[i].normal.y, new_region->line[i].d);
    }
    
    
    do {
      if(_keytest(RR_LEFT) && bl > 0) {
        --bl;
        while(_keytest(RR_LEFT)) ;
        break;
      }
      else if(_keytest(RR_RIGHT) && bl + 1 <= new_region->total_bl ) {
        ++bl;
        while(_keytest(RR_RIGHT)) ;
        break;
      }
    } while(1);
  } while(1);
  
  
  printf("clipped\n");
  
  LCD_restore(context->screen_data);
  
  while(1) ;
  
  /*
   *  randomize();
   *  
   *  Vex2D start, end;
   *  rand_line(&start, &end);
   *  
   *  //draw_line(start, end);
   *  
   *  // clip_edge(X3D_ClipData* clip, X3D_IndexedEdge* edge, X3D_ClippedEdge* edge_out)
   *  
   *  Vex2D points[] = {
   *    { -8192, -8192},
   *    { 8192, 8192 }
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








































