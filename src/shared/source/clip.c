// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

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


// Parameterizes the line between two points
void x3d_param_line2d(X3D_ParamLine2D* line, X3D_Vex2D_int16* a, X3D_Vex2D_int16* b) {
  int16 dx = b->x - a->x;
  int16 dy = b->y - a->y;

  // Calculate the normal for the line
  line->normal = (X3D_Vex2D_int16) { -dy, dx };

  // Calculate the distance to the origin
  /// @todo add overflow checking
  line->d = ((int32)line->normal.x * a->x + (int32)line->normal.y * a->y);
}



X3D_ClipRegion* x3d_construct_clipregion(X3D_Vex2D_int16* v, uint16 total_v) {
  uint16 i;

  X3D_ClipRegion* r = malloc(sizeof(X3D_ClipRegion) + sizeof(X3D_ParamLine2D) * total_v);

  for(i = 0; i < total_v; ++i) {
    uint16 next = (i + 1) % total_v;

    x3d_param_line2d(&r->pl[i], v + i, v + next);
  }

  r->total_pl = total_v;

  return r;
}

inline void get_edge(X3D_Prism2D* p, uint16 id, uint16* a, uint16* b) {
  if(id < p->base_v) {
    *a = id;

    if(id != p->base_v - 1)
      *b = id + 1;
    else
      *b = 0;
  }
  else if(id < p->base_v * 2) {
    *a = id;

    if(id != p->base_v * 2 - 1)
      *b = id + 1;
    else
      *b = p->base_v;
  }
  else {
    *a = id - p->base_v * 2;
    *b = id - p->base_v;
  }
}



#define SWAP(_a, _b) {typeof(_a) temp = _a; _a = _b; _b = temp;}

void x3d_prism2d_clip(X3D_Prism2D* prism, X3D_ClipRegion* clip, X3D_ViewPort* context) {
  // Check each point against each bounding line
  uint16 i, d;
  int32 dist[clip->total_pl][prism->base_v * 2];

  for(i = 0; i < clip->total_pl; ++i) {
    for(d = 0; d < prism->base_v * 2; ++d) {
      dist[i][d] = (int32)clip->pl[i].normal.x * prism->v[d].x + (int32)clip->pl[i].normal.y * prism->v[d].y - clip->pl[i].d;
      //printf("Dist: %ld\n", dist[i][d]);
      //ngetchx();
    }
  }

  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;    // Edge vertices
    uint16 clip_v;

    int16 a_min_scale = 0x7FFF;
    int16 b_min_scale = 0x7FFF;

    get_edge(prism, i, &a, &b);

    _Bool need_clip = 1;

    // Check each bounding line to see if a or b is outside
    for(d = 0; d < clip->total_pl; ++d) {

      _Bool a_out = dist[d][a] < 0;
      _Bool b_out = dist[d][b] < 0;

      if(a_out || b_out) {
        //printf("ERROR OUT\n");
        //ngetchx();
      }

      if(a_out != b_out) {
        if(b_out) {
          SWAP(a, b);
          SWAP(a_min_scale, b_min_scale);
        }

        int16 scale = ((int32)abs(dist[d][a]) << 8) / (abs(dist[d][b]) + abs(dist[d][a]));

        if(scale < a_min_scale) {
          a_min_scale = scale;
        }
      }
      else if(!a_out) {
        //X3D_Vex2D_int16 v_a = prism->v[a];
        //X3D_Vex2D_int16 v_b = prism->v[b];

        //x3d_draw_line_black(context, &v_a, &v_b);
        //need_clip = 0;
        //break;
        continue;
      }
      else {
        need_clip = 0;
        break;
      }
    }

    if(need_clip) {
      X3D_Vex2D_int16 v_a = prism->v[a];
      X3D_Vex2D_int16 v_b = prism->v[b];

      if(a_min_scale != 0x7FFF) {
        v_a.x += (((int32)prism->v[b].x - prism->v[a].x) * a_min_scale) >> 8;
        v_a.y += (((int32)prism->v[b].y - prism->v[a].y) * a_min_scale) >> 8;
        //printf("CLIP A\n");
      }

      if(b_min_scale != 0x7FFF) {
        v_b.x += (((int32)prism->v[a].x - prism->v[b].x) * b_min_scale) >> 8;
        v_b.y += (((int32)prism->v[a].y - prism->v[b].y) * b_min_scale) >> 8;
        //printf("CLIP B\n");
      }

      x3d_draw_line_black(context, &v_a, &v_b);
    }
  }
}

/// @todo document
void x3d_get_fail_planes(X3D_VertexClip* vc, X3D_Vex3D_int16* v, X3D_Frustum* f) {
  uint16 i;

  vc->total_fp = 0;

  for(i = 0; i < f->total_p; ++i) {
    int16 dot = x3d_vex3d_fp0x16_dot(v, &f->p[i].normal);

    // If the point is outside the plane, add the plane to the list
    if(dot < f->p[i].d) {
      vc->fp[vc->total_fp].dot = dot;
      vc->fp[vc->total_fp++].plane_d = f->p[i].d;
    }
  }
}

/// @todo document
void x3d_edge_clip(X3D_Edge* e, X3D_VertexClip* a, X3D_VertexClip* b, X3D_Frustum* f) {
  uint16 i, d;
  X3D_VertexClip* arr[2] = { a, b };
  X3D_VertexClip* v;
  X3D_VertexClip* v_other;

  for(i = 0; i < 2; ++i) {
    v = arr[i];
    v_other = arr[i ^ 1];

    for(d = 0; d < v->total_fp; ++d) {
      //fp8x8 t = div_int16_by_int16_as_fp8x8(v->fp[i].dot - v->fp[i].plane_d, v_other->fp[i]. );
    }
  }
}

void test_clip(X3D_ViewPort* context) {
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 10);
  X3D_VertexClip* clip_a = malloc(sizeof(X3D_VertexClip) + sizeof(X3D_FailPlane) * 10);

  X3D_Vex3D_int16 v_a = { 100, 0, 20 };

  x3d_frustum_from_rendercontext(frustum, context);
  x3d_get_fail_planes(clip_a, &v_a, frustum);


  printf("Fail planes:\n");

  uint16 i;
  for(i = 0; i < clip_a->total_fp; ++i) {
    //printf("%u\n", clip_a->fp[i].plane);
  }

  free(frustum);
}

inline void x3d_get_prism3d_edge(X3D_Prism3D* p, uint16 id, uint16* a, uint16* b) {
  if(id < p->base_v) {
    *a = id;

    if(id != p->base_v - 1)
      *b = id + 1;
    else
      *b = 0;
  }
  else if(id < p->base_v * 2) {
    *a = x3d_opposite_vertex(p, id - p->base_v);

    if(id != p->base_v * 2 - 1)
      *b = x3d_opposite_vertex(p, id + 1 - p->base_v);
    else
      *b = x3d_opposite_vertex(p, p->base_v - p->base_v);
  }
  else {
    *a = x3d_opposite_vertex(p, id - p->base_v * 2);
    *b = id - p->base_v * 2;
  }
}

/**
  Draws a wireframe 3D prism, clipped against a frustum.

  @param prism      - prism to draw
  @param frustum    - frustum to clip against
  @param context    - rendering context to draw to
  */
void x3d_draw_clipped_prism3d_wireframe(X3D_Prism* prism, X3D_Frustum* frustum, X3D_ViewPort* context, uint16 select_a, uint16 select_b) {
  //X3D_LOG_WAIT(context, "Planes: %d\n", frustum->total_p);

  uint16 i, d, vertex, plane, edge;
  const int TOTAL_V = prism->base_v * 2;
  uint8 was_projected[TOTAL_V];
  X3D_Vex2D_int16 project[TOTAL_V];
  X3D_Vex2D_int16 p[2];

  // A list for each vertex that holds which planes it's outside of
  uint16 outside[TOTAL_V][frustum->total_p];
  uint16 outside_total[TOTAL_V];

  uint16 outside_mask[TOTAL_V];

  // Calculate the distance between every point and every edge
  int16 dist[TOTAL_V][frustum->total_p];

  for(vertex = 0; vertex < TOTAL_V; ++vertex) {
    outside_total[vertex] = 0;
    outside_mask[vertex] = 0;
    was_projected[vertex] = 0;

    for(plane = 0; plane < frustum->total_p; ++plane) {
      dist[vertex][plane] = x3d_vex3d_fp0x16_dot(&frustum->p[plane].normal, &prism->v[vertex]) - frustum->p[plane].d;

      outside_mask[vertex] = (outside_mask[vertex] << 1) | (dist[vertex][plane] < 0);

      if(dist[vertex][plane] < 0) {
        outside[vertex][outside_total[vertex]++] = plane;

        //printf("Dist: %d\n", dist[vertex][plane]);
        //X3D_LOG_WAIT(context, "Neg!: %ld\n", dist[vertex][plane]);
      }
    }
  }

  //X3D_LOG_WAIT(context, "Dist");

  for(edge = 0; edge < prism->base_v * 3 + 1; ++edge) {
    uint16 a, b;

    if(edge < prism->base_v * 3)
      x3d_get_prism3d_edge(prism, edge, &a, &b);
    else {
      // Draw the selection spinner, if enabled
      if(select_a != select_b) {
        a = select_a;
        b = select_b;
      }
      else {
        break;
      }
    }

    // If two edges fail by the same plane, we can skip it
    if((outside_mask[a] & outside_mask[b]) != 0)
      continue;

    int16 min_scale[2] = { 0x7FFF, 0x7FFF };
    X3D_Vex3D_int16 clipped[2] = { prism->v[a], prism->v[b] };

    // If both points are inside all planes, the line is totally visible!
    if((outside_total[a] | outside_total[b]) == 0) {
      goto project_and_draw;
    }

#if 1
    for(vertex = 0; vertex < 2; ++vertex) {
      if(outside_total[a] != 0) {
        int16 min_scale_index = 0;

        for(i = 0; i < outside_total[a]; ++i) {
          //t = FIXDIV8(plane->d - dot, next_dot - dot);
          
          //int16 d = abs(dist[b][outside[a][i]]) + abs(dist[a][outside[a][i]]);
          //int16 n = abs(dist[b][outside[a][i]]);
          
          int16 n = abs(dist[b][outside[a][i]]);
          int16 d = abs(-dist[b][outside[a][i]] + frustum->p[outside[a][i]].d) + abs(-dist[a][outside[a][i]] + frustum->p[outside[a][i]].d);
          
          //printf("n: %d d: %d|", n, d);
          
          if(n == 0) {
            goto invisible;
          }
          
          //int16 d = abs(dist[b][outside[a][i]]) + abs(dist[a][outside[a][i]]);
          //int16 n = abs(dist[b][outside[a][i]]);

          if(n == d) {
            continue;
          }

          if(d < 1) {
            continue;
          }

          int32 scale = ((int32)n << 14) / (d);

          //printf("Scale: %d\n", scale);

          if(scale < min_scale[vertex]) {
            min_scale[vertex] = scale;
            min_scale_index = outside[vertex][i];
          }
        }

        if(min_scale[vertex] != 0x7FFF) {
          clipped[vertex].x = prism->v[b].x + ((((int32)prism->v[a].x - prism->v[b].x) * min_scale[vertex]) >> 14);
          clipped[vertex].y = prism->v[b].y + ((((int32)prism->v[a].y - prism->v[b].y) * min_scale[vertex]) >> 14);
          clipped[vertex].z = prism->v[b].z + ((((int32)prism->v[a].z - prism->v[b].z) * min_scale[vertex]) >> 14);
        }
      }

      SWAP(a, b);
    }
#endif

#if 1
    // If both of the points were outside, we need to check that the new clip is actually valid
    if(outside_total[a] != 0 && outside_total[b] != 0) {
      X3D_Vex3D_int16 mid = { (clipped[0].x + clipped[1].x) >> 1, (clipped[0].y + clipped[1].y) >> 1, (clipped[0].z + clipped[1].z) >> 1 };

      for(plane = 0; plane < frustum->total_p; ++plane) {
        if(x3d_vex3d_fp0x16_dot(&frustum->p[plane].normal, &mid) - frustum->p[plane].d < 0)
          continue;
      }
    }
#endif

project_and_draw:
    for(vertex = 0; vertex < 2; ++vertex) {

      if(was_projected[a] && 0) {
        if(outside_total[a] == 0) {
          p[vertex] = project[a];
        }
      }
      else {
        x3d_vex3d_int16_project(&p[vertex], &clipped[vertex], context);
        x3d_rendercontext_clamp_vex2d_int16(&p[vertex], context);

        if(outside_total[a] == 0) {
          was_projected[a] = 1;
          project[a] = p[vertex];
        }
      }

      SWAP(a, b);
    }

    x3d_draw_line_black(context, p, p + 1);
invisible:
  a = a;
  }

  //X3D_LOG_WAIT(context, "Done clipping\n");
}








// Gets a list of edge id's (in order) that make up a segment's face
uint16 x3d_prism3d_get_edges(X3D_Prism3D* prism, uint16 face, uint16 dest[]) {
  uint16 i;
  
  if(face == BASE_A) {
    for(i = 0; i < prism->base_v; ++i) {
      dest[i] = i;
    }

    return prism->base_v;
  }
  else if(face == BASE_B) {
    for(i = 0; i < prism->base_v; ++i) {
      dest[i] = i + prism->base_v;
    }

    return prism->base_v;
  }

  dest[0] = face - 2;
  dest[1] = face - 2 + prism->base_v;
  dest[2] = x3d_single_wrap(face - 2 + 1, prism->base_v) + prism->base_v;
  dest[3] = x3d_single_wrap(face - 2 + 1, prism->base_v);

  return 4;
}






#define EDGE_INVISIBLE 0xFF
#define EDGE_INSIDE 0
#define EDGE_CLIP_A 2
#define EDGE_CLIP_B 1
#define EDGE_CLIP_BOTH 3


//=============================================================================
/**
Draws a wireframe 3D prism, clipped against a frustum.

@param prism      - prism to draw
@param frustum    - frustum to clip against
@param context    - rendering context to draw to
*/
void x3d_draw_clipped_segment_solid(X3D_Segment* seg, X3D_Frustum* frustum, X3D_ViewPort* context, uint16 select_a, uint16 select_b) {
  //X3D_LOG_WAIT(context, "Planes: %d\n", frustum->total_p);
  X3D_Prism3D* prism = &seg->prism;


  uint16 i, d, vertex, plane, edge, face;
  const int TOTAL_V = prism->base_v * 2;
  uint8 was_projected[TOTAL_V];
  X3D_Vex2D_int16 project[TOTAL_V];
  X3D_Vex2D_int16 p[2];

  // A list for each vertex that holds which planes it's outside of
  uint16 outside[TOTAL_V][frustum->total_p];
  uint16 outside_total[TOTAL_V];

  uint16 outside_mask[TOTAL_V];

  // Calculate the distance between every point and every edge
  int16 dist[TOTAL_V][frustum->total_p];

  const uint16 total_e = prism->base_v * 3;

  uint8 edge_status[total_e];
  X3D_Edge clipped_edges[total_e];


  for(vertex = 0; vertex < TOTAL_V; ++vertex) {
    outside_total[vertex] = 0;
    outside_mask[vertex] = 0;
    was_projected[vertex] = 0;

    for(plane = 0; plane < frustum->total_p; ++plane) {
      dist[vertex][plane] = x3d_vex3d_fp0x16_dot(&frustum->p[plane].normal, &prism->v[vertex]) - frustum->p[plane].d;

      outside_mask[vertex] = (outside_mask[vertex] << 1) | (dist[vertex][plane] < 0);

      if(dist[vertex][plane] < 0) {
        outside[vertex][outside_total[vertex]++] = plane;

        //printf("Dist: %d\n", dist[vertex][plane]);
        //X3D_LOG_WAIT(context, "Neg!: %ld\n", dist[vertex][plane]);
      }
    }
  }

  //X3D_LOG_WAIT(context, "Dist");

  for(edge = 0; edge < total_e; ++edge) {
    uint16 a, b;

    if(edge < prism->base_v * 3)
      x3d_get_prism3d_edge(prism, edge, &a, &b);
    else {
      // Draw the selection spinner, if enabled
      if(select_a != select_b) {
        a = select_a;
        b = select_b;
      }
      else {
        break;
      }
    }

    // If two edges fail by the same plane, we can skip it
    if((outside_mask[a] & outside_mask[b]) != 0) {
      edge_status[i] = EDGE_INVISIBLE;
      continue;
    }

    int16 min_scale[2] = { 0x7FFF, 0x7FFF };
    X3D_Vex3D_int16 clipped[2] = { prism->v[a], prism->v[b] };

    // If both points are inside all planes, the line is totally visible!
    if((outside_total[a] | outside_total[b]) == 0) {
      edge_status[i] = EDGE_INSIDE;
      goto project_and_draw;
    }

    edge_status[i] = 0;

#if 1
    for(vertex = 0; vertex < 2; ++vertex) {
      edge_status[i] = (edge_status[i] << 1) | (outside_total[a] != 0);

      if(outside_total[a] != 0) {
        int16 min_scale_index = 0;

        for(i = 0; i < outside_total[a]; ++i) {
          //t = FIXDIV8(plane->d - dot, next_dot - dot);
          
          //int16 d = abs(dist[b][outside[a][i]]) + abs(dist[a][outside[a][i]]);
          //int16 n = abs(dist[b][outside[a][i]]);
          
          int16 n = abs(dist[b][outside[a][i]]);
          int16 d = abs(-dist[b][outside[a][i]] + frustum->p[outside[a][i]].d) + abs(-dist[a][outside[a][i]] + frustum->p[outside[a][i]].d);
          

          if(n == d)
            continue;

          if(d < 1)
            continue;

          int16 scale = ((int32)n << 8) / (d);

          //printf("Scale: %d\n", scale);

          if(scale < min_scale[vertex]) {
            min_scale[vertex] = scale;
            min_scale_index = outside[vertex][i];
          }
        }

        if(min_scale[vertex] != 0x7FFF) {
          clipped[vertex].x = prism->v[b].x + ((((int32)prism->v[a].x - prism->v[b].x) * min_scale[vertex]) >> 8);
          clipped[vertex].y = prism->v[b].y + ((((int32)prism->v[a].y - prism->v[b].y) * min_scale[vertex]) >> 8);
          clipped[vertex].z = prism->v[b].z + ((((int32)prism->v[a].z - prism->v[b].z) * min_scale[vertex]) >> 8);
        }
      }

      SWAP(a, b);
    }
#endif

#if 1
    // If both of the points were outside, we need to check that the new clip is actually valid
    if(outside_total[a] != 0 && outside_total[b] != 0) {
      X3D_Vex3D_int16 mid = { (clipped[0].x + clipped[1].x) >> 1, (clipped[0].y + clipped[1].y) >> 1, (clipped[0].z + clipped[1].z) >> 1 };

      for(plane = 0; plane < frustum->total_p; ++plane) {
        if(x3d_vex3d_fp0x16_dot(&frustum->p[plane].normal, &mid) - frustum->p[plane].d < 0) {
          edge_status[i] = EDGE_INVISIBLE;
          continue;
        }
      }
    }
#endif

project_and_draw:
    for(vertex = 0; vertex < 2; ++vertex) {

      if(was_projected[a] && 0) {
        if(outside_total[a] == 0) {
          p[vertex] = project[a];
        }
      }
      else {
        x3d_vex3d_int16_project(&p[vertex], &clipped[vertex], context);
        x3d_rendercontext_clamp_vex2d_int16(&p[vertex], context);

        if(outside_total[a] == 0) {
          was_projected[a] = 1;
          project[a] = p[vertex];
        }
      }

      SWAP(a, b);
    }

    x3d_draw_line_black(context, p, p + 1);
  }


  // Ok, now we have all the clipping data. Now we need to reconstruct the viewing frustum for each face
  // that has another segment attached to it.
  const uint16 TOTAL_F = prism->base_v + 2;
  uint16 edge_list[prism->base_v];
  uint16 edges;
  X3D_Plane planes[prism->base_v];
  //uint16 planes;

  for(face = 0; face < TOTAL_F; ++i) {
    // For right now, we're only going to try this construction with BASE_A
    if(face == BASE_A) {
      edges = x3d_prism3d_get_edges(prism, face, edge_list);

      uint16 edge = 0xFFFF;
      // First, find an edge that is not totally invisible
      for(i = 0; i < edges; ++i) {
        if(edge_status[i] != EDGE_INVISIBLE) {
          edge = i;
          break;
        }
      }

      //planes = 0;

      uint16 edge_pos = 0;

      if(edge != 0xFFFF) {
        do {



        } while(1);


















      }
      else {
        // Yuck, we have the case where none of the edges are visible
        // To be worked out later...
      }
    }
  }
}

//=============================================================================
// Old clipping code
//=============================================================================


#define FIXDIV8(_n, _d) (((long)(_n) << 14) / (_d))

#define FIXMULN(_a, _b, _n) (((long)(_a) * (_b)) >> (_n))




#define FIXMUL8(_a, _b) FIXMULN(_a, _b, 14)
#define FIXMUL15(_a, _b) FIXMULN(_a, _b, 15)

// Clips a polygon against a plane. Returns whether a valid polygon remains.
_Bool x3d_clip_polygon3d_to_plane(X3D_Polygon3D* poly, X3D_Plane* plane, X3D_Polygon3D* dest) {
  int16 i;
  int16 next_point;
  int16 in, next_in;
  int16 dot, next_dot;
  int16 t;

  int16 out[30];
  short out_pos = 0;

  dot = x3d_vex3d_fp0x16_dot(&poly->v[0], &plane->normal);
  in = (dot >= plane->d);

  //printf("Dot: %d\nD: %d\n", dot, plane->d);
  //print_vex3d(&plane->normal);
  //print_vex3d(&poly->v[0]);
  //ngetchx();

  int16 total_outside = !in;

  dest->total_v = 0;

  int16 clipped = 0;

  for(i = 0; i < poly->total_v; i++) {
#if 1
    if(clipped == 2) {
      // A convex polygon can at most have two edges clipped, so if we've reached it
      // just copy over the other ones (assuming we're back to being inside the poly)

      if(in) {

        for(; i < poly->total_v; i++) {
          dest->v[dest->total_v++] = poly->v[i];
        }
      }

      break;

      //plane_clipped_saved++;
    }
#endif


    next_point = i + 1;
    if(next_point == poly->total_v)
      next_point = 0;


    // The vertex is inside the plane, so don't clip it
    if(in) {
      dest->v[dest->total_v++] = poly->v[i];
    }

    //errorif(!in, "Point not in!");


    next_dot = x3d_vex3d_fp0x16_dot(&poly->v[next_point], &plane->normal);
    next_in = (next_dot >= plane->d);

    total_outside += !next_in;

    //	printf("Next dot: %d\n", next_dot);

    // The points are on opposite sides of the plane, so clip it
    if(in != next_in) {
      ++clipped;

      // Scale factor to get the point on the plane
      // errorif((long)next_dot - dot == 0, "Clip div by 0");

      t = FIXDIV8(plane->d - dot, next_dot - dot);
      //t = fast_div_fix(plane->d - dot, next_dot - dot, 15 - 8);


      //errorif(abs((long)plane->d - dot) > 32767, "plane->d too big");
      //errorif(abs((long)next_dot - dot) > 32767, "next_dot too big");

      //errorif(abs(t) > 32767, "Invalid clip t");
      //errorif(t == 0, "t == 0");

      if(t == 0) {
        //printf("T == 0\n");
      }

      //if(cube_id == 12) {
        //printf("T: %ld\n", t);
      //}

      //printf("Dist: %d\n", dot + plane->d);
      //printf("T: %ld Z: %d\n", t, poly->v[i].z);

      dest->v[dest->total_v].x = poly->v[i].x + FIXMUL8(((long)poly->v[next_point].x - poly->v[i].x), t);
      dest->v[dest->total_v].y = poly->v[i].y + FIXMUL8(((long)poly->v[next_point].y - poly->v[i].y), t);
      dest->v[dest->total_v].z = poly->v[i].z + FIXMUL8(((long)poly->v[next_point].z - poly->v[i].z), t);






      // Use floats to make sure we're not overflowing





      //dest->draw[dest->total_v] = poly->draw[i];

      //printf("Dest z: %d\n", dest->v[dest->total_v].z);
      //printf("Should be: %d\n", -plane->d);

      //errorif(dest->v[dest->total_v].z < DIST_TO_NEAR_PLANE / 2, "Invalid clip: %d", dest->v[dest->total_v].z);

      dest->total_v++;
    }

    if(next_in != in) {
      out[out_pos++] = dest->total_v - 1;
    }

    dot = next_dot;
    in = next_in;
  }

  //printf("total outside: %d\ntotal: %d", total_outside, dest->total_v);

  if(dest->total_v > 4) {
    //clrscr();

    int i;

    for(i = 0; i < dest->total_v; i++) {
      //	print_vex3d(&dest->v[i]);
    }

    //Polygon2D out;

  }

  //===============================
  //for(i = 0; i < out_pos - 1; i++) {
  //  if(out[i] == out[i + 1] - 1) {
  //    //printf("Case\n");
  //    dest->draw[out[i]] = 0;
  //  }
  //  else {
  //    //error("ERRORX");

  //    if(out[i] != 0 || out[i + 1] != dest->total_v - 1) {

  //      printf("A: %d, B: %d\n", out[i], out[i + 1]);

  //      dest->draw[out[i]] = 0;
  //      dest->draw[out[i + 1]] = 0;
  //    }


  //  }
  //}

  //if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_v - 1) {
  //  dest->draw[dest->total_v - 1] = 0;
  //  //printf("CASE\n");
  //}




  //if(out_pos != 0 && out_pos != 2) {
  //  //printf("CASE<----\n");
  //  //error("Wrong out pos\n");
  //  invert_screen = 1;
  //  return 0;


  //  PortRestore();
  //  clrscr();

  //  print_polygon(poly);
  //  printf("Total: %d\n", out_pos);

  //  while(1);
  //}



  //errorif(out_pos != 0 && out_pos != 2, "Wrong out pos: %d\n", out_pos);

  //if(cube_id == 14) {
  //  //printf("Total outside: %d\n", total_outside);
  //}

  //xassert(dest->total_v < MAX_POINTS);

  //printf("\nV: %d\n", dest->total_v);

  return dest->total_v > 1;
}

// Clips a polygon against the entire view frustum
// This routine requires two temporary polygons, one of which the
// final polygon will be in. This returns the address of which one it
// is
_Bool x3d_clip_polygon_to_frustum(X3D_Polygon3D* src, X3D_Frustum* f, X3D_Polygon3D* dest) {
#if 1
  X3D_Polygon3D* temp[2] = { ALLOCA_POLYGON3D(30), ALLOCA_POLYGON3D(30) };
  int16 current_temp = 0;
  X3D_Polygon3D* poly = src;
  int16 i;


  //xassert(f->total_p != 0);

  //xassert(f->total_p < MAX_PLANES);

  if(f->total_p == 0)
    return 0;


#if 1
  for(i = 0; i < f->total_p - 1; i++) {
    //errorif(poly->total_v < 3, "Invalid clip poly");
    //return clip_polygon_to_plane(src, &f->p[FRUSTUM_TOP], dest);

    //if(i == PLANE_LEFT || i == PLANE_RIGHT)
    //	continue;

    if(!x3d_clip_polygon3d_to_plane(poly, &f->p[i], temp[current_temp])) {
      dest->total_v = 0;
      return 0;
    }

    poly = temp[current_temp];
    current_temp = !current_temp;
  }
#endif

  //return poly->total_v > 2;
  return x3d_clip_polygon3d_to_plane(poly, &f->p[f->total_p - 1], dest);

#endif
}

