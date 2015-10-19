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
#include "X3D_newclip.h"

X3D_ClipReport report;

/**
 * Calculates the signed distance from a point to a bounding line. This
 *    distance is positive if on the normal-facing side of the line, and
 *    negative otherwise.
 * 
 * @param line    - bounding line
 * @param v       - point to test
 * 
 * @return Distance to the bounding line.
 */
inline int16 x3d_dist_to_line(X3D_BoundLine* line, Vex2D* v) {
  report.mul += 2;
  
  
  return ((((int32)line->normal.x * v->x) + ((int32)line->normal.y * v->y)) >> X3D_NORMAL_SHIFT) + line->d;
}

/**
 * Calculates a bounding line between @ref a and @ref b. It is assumed that
 *    @ref a and @ref b both lie on a clockwise convex polygon, which affects
 *    which way the normal points.
 * 
 * @param line      - output line
 * @param a         - first point
 * @param b         - second point
 * 
 * @return Nothing.
 */
void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
  Vex2D normal = { -(b->y - a->y), b->x - a->x };
  Vex2D new_normal = normal;

  x3d_normalize_vex2d_fp0x16(&new_normal);

  line->normal = new_normal;
  line->d = (((int32)-line->normal.x * a->x) - ((int32)line->normal.y * a->y)) >> X3D_NORMAL_SHIFT;

  line->v[0] = *a;
  line->v[1] = *b;
  
  report.bound_line++;
}

/**
 * Gets a list of all edge indexes (in order) that make up @ref face from a
 *    2D prism.
 * 
 * @param prism   - 2D prism
 * @param face    - face ID to get edges for
 * @param edges   - output list of edges
 * 
 * @return The number of edges that comprise the face.
 */
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
  } else{
    face -= 2;
    edges[0] = x3d_single_wrap(face + 1, prism->base_v) + prism->base_v * 2;
    edges[1] = face + prism->base_v;
    edges[2] = face + prism->base_v * 2;
    edges[3] = face;
    
    return 4;
  }
}

void x3d_construct_boundregion(X3D_BoundRegion* region, Vex2D v[], uint16 total_v) {
  int16 i;
  
  if(x3d_is_clockwise_turn(v, v + 1, v + 2)) {
    for(i = 0; i < total_v; ++i) {
      x3d_construct_boundline(region->line + i, v + i, v + ((i + 1) % total_v));
    }
  }
  else {
    for(i = total_v - 1; i >= 0; --i) {
      x3d_construct_boundline(region->line + i, v + i, (i != 0 ? v + i - 1 : v + total_v - 1));
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

#define INDEX(_clip, _line, _v) _line * _clip->region->total_bl + _v

#define DIST(_clip, _line, _v) _clip->dist[INDEX(_clip, _line, _v)]

#define OUTSIDE(_clip, _v, _pos) _clip->dist[INDEX(_clip, _pos, _v)]

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

#define FIXED_ARR_SIZE
#define ARR_SIZE 64

#define BL_ARR_SIZE 16

static inline int16* dist_ptr(X3D_ClipData* clip, uint16 line, uint16 vertex) {
#ifndef FIXED_ARR_SIZE
  report.arr2D++;
  
  return clip->line_dist + line * clip->total_v + vertex;
#else
  return clip->line_dist + line * (ARR_SIZE / sizeof(uint16)) + vertex;
#endif
}

static inline int16* outside_ptr(X3D_ClipData* clip, uint16 vertex, uint16 pos) {
#ifndef FIXED_ARR_SIZE
  report.arr2D++;
  
  return clip->outside + vertex * clip->region->total_bl + pos;
#else
  return clip->outside + vertex * (BL_ARR_SIZE / sizeof(uint16)) + pos;
#endif
}

static inline void add_outside(X3D_ClipData* clip, uint16 vertex, uint16 line) {
  *outside_ptr(clip, vertex, clip->outside_total[vertex]++) = line;
}

static inline int16 dist(X3D_ClipData* clip, uint16 line, uint16 vertex) {
  int16 n = *dist_ptr(clip, line, vertex);
  
  return n;  
}

typedef struct X3D_BoundRect {
  Vex2D top_left;
  Vex2D bottom_right;
} X3D_BoundRect;

void construct_boundrect(Vex2D* a, Vex2D* b, X3D_BoundRect* rect) {
  if(a->x < b->x) {
    rect->top_left.x = a->x;
    rect->bottom_right.x = b->x;
  }
  else {
    rect->top_left.x = b->x;
    rect->bottom_right.x = a->x;
  }
  
  if(a->y < b->y) {
    rect->top_left.y = a->y;
    rect->bottom_right.y = b->y;
  }
  else {
    rect->top_left.y = b->y;
    rect->bottom_right.y = a->y;
  }    
  
}
  
  

_Bool boundline_boundrect_intersect(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
}



static inline void calc_line_distances(X3D_ClipData* clip) {
  uint16 vertex, line;
  
  for(vertex = 0; vertex < clip->total_v; ++vertex) {
    uint32 outside_mask = 0;
    
    for(line = 0; line < clip->region->total_bl; ++line) {
      
      int16 d = x3d_dist_to_line(clip->region->line + line, clip->v + vertex);
      *dist_ptr(clip, line, vertex) = d;
      
      outside_mask = (outside_mask << 1) | (d <= 0 ? 1 : 0);
      
      if(d <= 0) {
        add_outside(clip, vertex, line);      
      }
    }
    
    clip->outside_mask[vertex] = outside_mask;
  }
}

static inline int16 clip_scale(X3D_ClipData* clip, uint16 start, uint16 end, uint16 line) {
  int16 n = dist(clip, line, start);
  int16 d = abs(dist(clip, line, end)) + abs(dist(clip, line, start));
  
  if(n <= 0)
    return 0;
  
  if(n == d)
    return 0x7FFF;
  
  ++report.div;
  
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
  
  report.mul += 2;
  
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
  
  if((clip->outside_mask[edge->v[0]] & clip->outside_mask[edge->v[1]]) != 0) {
    goto invisible;
  }
    
  for(vex = 0; vex < 2; ++vex) {
    
    if(!vertex_visible(clip, edge->v[vex])) {
      uint16 min_scale_line;
      int16 scale = min_clip_scale(clip, edge->v[vex ^ 1], edge->v[vex], &min_scale_line);
      
      if(scale != 0) {
        scale_edge(&edge_out->v[vex].v, &clip->v[edge->v[vex ^ 1]], &clip->v[edge->v[vex]], scale);
        edge_out->v[vex].clip_line = min_scale_line;
        edge_out->v[vex].clip_status = CLIP_CLIPPED;
      }
      else {
invisible:
        edge_out->v[0].clip_status = CLIP_INVISIBLE;
        edge_out->v[1].clip_status = CLIP_INVISIBLE;
        
        edge_out->v[0].v = clip->v[edge->v[0]];
        edge_out->v[1].v = clip->v[edge->v[1]];
        
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
    
    if(new_dx == 0)
      new_dx = SIGNOF(old_dx);
    
    if(new_dy == 0)
      new_dy = SIGNOF(old_dy);
    
    if(SIGNOF(new_dx) != SIGNOF(old_dx) || SIGNOF(new_dy) != SIGNOF(old_dy)) {
      edge_out->v[0].clip_status = CLIP_INVISIBLE;
      edge_out->v[1].clip_status = CLIP_INVISIBLE;
      edge_out->v[0].v = clip->v[edge->v[0]];
      edge_out->v[1].v = clip->v[edge->v[1]];
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
  
#ifndef FIXED_ARR_SIZE
  clip->line_dist = alloca(sizeof(int16) * clip->total_v * clip->region->total_bl);
  clip->outside = alloca(sizeof(uint16) * clip->total_v * clip->region->total_bl);
  
#else
  
  clip->line_dist = alloca(ARR_SIZE * clip->region->total_bl);
  clip->outside = alloca(BL_ARR_SIZE * clip->total_v);
#endif
  
  
  clip->outside_total = alloca(sizeof(uint16) * clip->total_v);
  
  uint16 i;
  for(i = 0; i < clip->total_v; ++i)
    clip->outside_total[i] = 0;
  
  // Calculate the distance from every point to every line
  calc_line_distances(clip);
  
  //printf("Total e: %d\n", clip->total_e);
  
  for(i = 0; i < clip->total_e; ++i)
    clip_edge(clip, clip->edge + i, clip->edge_clip + i);
  
}

X3D_ViewPort* global_viewport;

void draw_line(Vex2D a, Vex2D b) {
  //DrawLine(a.x, a.y, b.x, b.y, A_NORMAL);
  x3d_draw_line_black(global_viewport, &a, &b);
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
    
    for(i = 0; i < total_e; ++i) {
      reverse_edge_list[i] = edge_list[total_e - i - 1];
      
      SWAP(EDGE(i).v[0], EDGE(i).v[1]);
    }
    
    edge_list = reverse_edge_list;
    
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
          x3d_construct_boundline(region->line + region->total_bl, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v);
          
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
        
        
        x3d_construct_boundline(region->line + region->total_bl, &EDGE(edge_id).v[0].v, &EDGE(edge_id).v[1].v);
        
        if(region->total_bl == 0 || diff_boundline(region->line + region->total_bl, region->line + region->total_bl - 1))
          ++region->total_bl;
      }
      
      edge_id = x3d_single_wrap(edge_id + 1, total_e);
    }
    
    
  }
  else {
    uint16 i;
    uint32 edge_mask = (1L << clip->region->total_bl) - 1;// = clip->outside_mask[clip->edge->v[0]];
    
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
        if(x3d_is_clockwise_turn(&EDGE(i).v[0].v, &clip->region->point_inside, &EDGE(i).v[1].v)) {
          result_region = NULL;
          break;
        }
      }
      
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
  
  if(face > 1) {
    SWAP(clip->edge[edges[0]].v[0], clip->edge[edges[0]].v[1]);
    SWAP(clip->edge_clip[edges[0]].v[0], clip->edge_clip[edges[0]].v[1]);
  }
  
  uint16 v[3] = {
    clip->edge[edges[0]].v[0],
    clip->edge[edges[1]].v[0],
    clip->edge[edges[2]].v[0]
  };
  
  _Bool clockwise = x3d_is_clockwise_turn(clip->v + v[0], clip->v + v[1], clip->v + v[2]);
  
  X3D_BoundRegion* result = x3d_construct_boundregion_from_clip_data(clip, edges, total_e, region, clockwise);
  
  if(face > 1) {
    SWAP(clip->edge[edges[0]].v[0], clip->edge[edges[0]].v[1]);
    SWAP(clip->edge_clip[edges[0]].v[0], clip->edge_clip[edges[0]].v[1]);
  }
  
  return result;
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

void draw_prism2d_edge(X3D_Prism2D* prism, uint16 id) {
  uint16 a, b;
  
  x3d_get_prism2d_edge(prism, id, &a, &b);
  
  draw_line(prism->v[a], prism->v[b]);
}

void clip_prism2d_to_boundregion(X3D_Prism2D* prism, X3D_BoundRegion* region, X3D_ClipData* clip) {
  clip->region = region;
  clip->total_v = prism->base_v * 2;
  clip->v = prism->v;
  clip->total_e = prism->base_v * 3;
  
  generate_prism2d_edge_list(prism, clip->edge);
  x3d_clip_edges(clip);
}

void draw_clipped_edges(X3D_ClipData* clip) {
  uint16 i;
  
  for(i = 0; i < clip->total_e; ++i) {
    if(clip->edge_clip[i].v[0].clip_status != CLIP_INVISIBLE) {
      draw_line(clip->edge_clip[i].v[0].v, clip->edge_clip[i].v[1].v);
    }
  }
}

void draw_prism2d(X3D_Prism2D* prism2d) {
  uint16 i;
  
  for(i = 0; i < prism2d->base_v * 3; ++i) {
    uint16 a, b;
    
    x3d_get_prism2d_edge(prism2d, i, &a, &b);
    
    draw_line(prism2d->v[a], prism2d->v[b]);
  }
}

void draw_polygon(Vex2D* p, uint16 total_p) {
  uint16 i;
  
  for(i = 0; i < total_p; ++i) {
    uint16 next = (i + 1) % total_p;
    draw_line(p[i], p[next]);
  }
}

uint16 get_num() {
  char buf[20];
  uint16 pos = 0;
  
  uint16 keys[10][2] = {
    {RR_0},
    {RR_1},
    {RR_2},
    {RR_3},
    {RR_4},
    {RR_5},
    {RR_6},
    {RR_7},
    {RR_8},
    {RR_9},
  };
  
  uint16 i;
  
  do {
    for(i = 0; i < 10; ++i) {
      if(_keytest(keys[i][0], keys[i][1])) {
        while((_keytest(keys[i][0], keys[i][1]))) ;
        
        buf[pos] = i + '0';
        printf("%c", buf[pos]);
        buf[++pos] = '\0';
      }
    }
    
    if(_keytest(RR_ENTER)) {
      return atoi(buf);
    }
    
  } while(1);
  
}

void test_clip_scale(X3D_Context* context, X3D_ViewPort* port) {
  Vex2D clip_v[20];  
  uint16 i, d;
  
  X3D_BoundRegion* region = malloc(8000);//alloca(sizeof(X3D_BoundLine) * 20 + sizeof(X3D_BoundRegion));
  
  X3D_ClipData clip;
  
  clip.line_dist = malloc(8000);
  clip.outside = malloc(8000);
  clip.outside_total = malloc(8000);
  clip.edge_clip = malloc(8000);
  clip.edge = malloc(8000);
  clip.outside_mask = malloc(8000);
  
  Vex2D* p = clip_v;
  
  for(i = 0; i < 20; ++i)
    clip.outside_total[i] = 0;
  
  X3D_Prism3D* prism = malloc(8000);//alloca(100);  
  
  x3d_prism_construct(prism, 8, 180, 50, (Vex3D_angle256) { ANG_180, ANG_45, 0 });
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    prism->v[i].z += 400;
  }
  
  
  X3D_Prism2D* prism2d = malloc(8000);//alloca(500);
  
  TEST_x3d_project_prism3d(prism2d, prism, port);
  
  for(i = 0; i < prism2d->base_v * 2; ++i) {
    //prism2d->v[i].x += 20;
  }
  
  clip.total_e = prism2d->base_v * 3;
  
  clrscr();
  draw_prism2d(prism2d);
  
  uint16 total_clip_v = input_polygon2d(clip_v);
  
  x3d_construct_boundregion(region, p, total_clip_v);
  draw_polygon(p, total_clip_v);
  
  clrscr();
  clip_prism2d_to_boundregion(prism2d, region, &clip);
  draw_polygon(p, total_clip_v);
  draw_clipped_edges(&clip);
  
  X3D_BoundRegion* new_region = alloca(sizeof(X3D_BoundLine) * 12 + sizeof(X3D_BoundRegion));
  
  //char input[3];
  //printf("Base (0 - %d): ", prism2d->base_v + 2 - 1);
  //uint16 face = get_num();
  
  
  X3D_BoundRegion* old_region = new_region;
  
  for(i = 0; i < prism2d->base_v + 2; ++i) {
    new_region = old_region;
    new_region = x3d_construct_boundregion_from_prism2d_face(&clip, prism2d, i, new_region);
    
    if(new_region == NULL) {
      printf("INVISIBLE\n");
      continue;
    }
    printf("Bl: %d\n", new_region->total_bl);
    
    //while(!_keytest(RR_W)) ;
    
    clrscr();
    draw_polygon(p, total_clip_v);
    draw_prism2d(prism2d);
    fill_boundregion(new_region);
    draw_polygon(p, total_clip_v);
    draw_prism2d(prism2d);
  }
  
  while(1) ;
}





void x3d_test_new_clip(X3D_Context* context, X3D_ViewPort* port) {
  test_clip_scale(context, port);
  return;
}


static inline uint16 x3d_prism3d_needed_size(uint16 base_v) {
  return sizeof(X3D_Prism3D) + sizeof(Vex3D) * base_v * 2;
}

static inline uint16 x3d_prism2d_needed_size(uint16 base_v) {
  return sizeof(X3D_Prism2D) + sizeof(Vex2D) * base_v * 2;
}

void x3d_test_rotate_prism3d(X3D_Prism* dest, X3D_Prism* src, X3D_Camera* cam);


void x3d_draw_clip_segment(uint16 id, X3D_BoundRegion* region, X3D_Context* context, X3D_ViewPort* viewport) {
  global_viewport = viewport;
  
  X3D_Segment* seg = x3d_get_segment(context, id);
  X3D_Prism3D* prism_temp = alloca(x3d_prism3d_needed_size(seg->prism.base_v));
  X3D_Prism2D* prism2d = alloca(x3d_prism2d_needed_size(seg->prism.base_v));
  
  
  x3d_test_rotate_prism3d(prism_temp, &seg->prism, context->cam);
  TEST_x3d_project_prism3d(prism2d, prism_temp, viewport);
  
  X3D_ClipData clip;
  
  clip.total_v = prism2d->base_v * 2;
  clip.edge = alloca(sizeof(X3D_IndexedEdge) * (prism2d->base_v * 3));
  clip.total_e = prism2d->base_v * 3;
  clip.edge_clip = alloca(sizeof(X3D_ClippedEdge) * clip.total_e);
  clip.outside_mask = alloca(sizeof(uint32) * clip.total_v * 2);
  
  clip_prism2d_to_boundregion(prism2d, region, &clip);
  
  
  //draw_prism2d(prism2d);
  
  draw_clipped_edges(&clip);
  //draw_prism2d(prism2d);
  
  uint16 i;
  
  if(id != 0)
    return;
  
  X3D_SegmentFace* face = x3d_segment_get_face(seg);
  
  for(i = 0; i < x3d_segment_total_f(seg); ++i) {
    if(face[i].connect_id != SEGMENT_NONE) {
      X3D_BoundRegion* new_region = alloca(sizeof(X3D_BoundRegion) + sizeof(X3D_BoundLine) * 10);
      
      new_region = x3d_construct_boundregion_from_prism2d_face(&clip, prism2d, i, new_region);
      
      if(new_region != NULL) {
        x3d_draw_clip_segment(face[i].connect_id, new_region, context, viewport);
      }
    }
  }
  
  
  
  //x3d_draw_clipped_prism3d_wireframe(prism_temp, frustum, viewport, 0, 0);
}






































