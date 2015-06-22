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

