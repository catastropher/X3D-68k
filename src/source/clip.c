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


// Parameterizes the line between two points
void x3d_param_line2d(X3D_ParamLine2D* line, X3D_Vex2D_int16* a, X3D_Vex2D_int16* b) {
  int16 dx = b->x - a->x;
  int16 dy = b->y - a->y;
  
  // Calculate the normal for the line
  line->normal = (X3D_Vex2D_int16){ -dy, dx };
  
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

void x3d_prism2d_clip(X3D_Prism2D* prism, X3D_ClipRegion* clip, X3D_RenderContext* context) {
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

void test_clip(X3D_RenderContext* context) {
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

typedef X3D_Prism X3D_Prism3D;

inline void x3d_get_prism3d_edge(X3D_Prism3D* p, uint16 id, uint16* a, uint16* b) {
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

/**
  Draws a wireframe 3D prism, clipped against a frustum.
  
  @param prism      - prism to draw
  @param frustum    - frustum to clip against
  @param context    - rendering context to draw to
*/
void x3d_draw_clipped_prism3d_wireframe(X3D_Prism* prism, X3D_Frustum* frustum, X3D_RenderContext* context) {
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
        
        //printf("Dist: %ld\n", dist[vertex][plane]);
        //X3D_LOG_WAIT(context, "Neg!: %ld\n", dist[vertex][plane]);
      }
    }
  }

  //X3D_LOG_WAIT(context, "Dist");

  for(edge = 0; edge < prism->base_v * 3; ++edge) {
    uint16 a, b;
    x3d_get_prism3d_edge(prism, edge, &a, &b);

    // If two edges fail by the same plane, we can skip it
    if((outside_mask[a] & outside_mask[b]) != 0)
      continue;

    int16 min_scale[2] = { 0x7FFF, 0x7FFF };
    X3D_Vex3D_int16 clipped[2] = { prism->v[a], prism->v[b] };

    // If both points are inside are planes, the line is totally visible!
    if((outside_total[a] | outside_total[b]) == 0) {
      goto project_and_draw;
    }

#if 1
    for(vertex = 0; vertex < 2; ++vertex) {
      if(outside_total[a] != 0) {
        int16 min_scale_index = 0;

        for(i = 0; i < outside_total[a]; ++i) {
          int16 d = abs(dist[b][outside[a][i]]) + abs(dist[a][outside[a][i]]) + 1;

          

          if(d < 2)
            continue;

          int16 scale = ((int32)abs(dist[a][outside[a][i]]) << 15) / (d);

          //printf("Scale: %d\n", scale);

          if(scale < min_scale[vertex]) {
            min_scale[vertex] = scale;
            min_scale_index = outside[vertex][i];
          }
        }

        clipped[vertex].x += (((int32)prism->v[b].x - prism->v[a].x) * min_scale[vertex]) >> 15;
        clipped[vertex].y += (((int32)prism->v[b].y - prism->v[a].y) * min_scale[vertex]) >> 15;
        clipped[vertex].z += (((int32)prism->v[b].z - prism->v[a].z) * min_scale[vertex]) >> 15;
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

      if(was_projected[a]) {
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

  //X3D_LOG_WAIT(context, "Done clipping\n");
}


















