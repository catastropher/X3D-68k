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

