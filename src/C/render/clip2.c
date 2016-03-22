// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_screen.h"
#include "X3D_keys.h"
#include "X3D_clip.h"
#include "X3D_assert.h"
#include "X3D_enginestate.h"
#include "X3D_trig.h"
#include "render/X3D_scanline.h"

//#define x3d_log(...) ;


_Bool x3d_points_clockwise(X3D_PolyVertex* a, X3D_PolyVertex* b, X3D_PolyVertex* c) {
  int32 t1 = ((int32)b->v2d.x - a->v2d.x) * (c->v2d.y - a->v2d.y);
  int32 t2 = ((int32)c->v2d.x - a->v2d.x) * (b->v2d.y - a->v2d.y);
  
  return t1 - t2 < 0;
}


void x3d_polyline_draw(X3D_PolyLine* p, X3D_Color c) {
  uint16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
    uint16 next = (i + 1 < p->total_v ? i + 1 : 0);
    
    x3d_screen_draw_line(p->v[i]->v2d.x, p->v[i]->v2d.y, p->v[next]->v2d.x, p->v[next]->v2d.y, c);
  }
}

fp16x16 x3d_val_slope(int16 d_a, int16 d_b) {
  if(d_b == 0) return 0;
  return ((int32)d_a << 16) / d_b;
}

int16 x3d_line_parametric_t(X3D_Vex2D* a, X3D_Vex2D* b, X3D_Vex2D* v) {
  int16 dx = abs(v->x - a->x);
  int16 dy = abs(v->y - a->y);
  
  // Calculate t using whichever variable has a larger difference, to increase
  // accuracy
  if(dx > dy) {
    return (((int32)v->x - a->x) << 15) / (b->x - a->x);
  }
  else {
    return (((int32)v->y - a->y) << 15) / (b->y - a->y);
  }
}

int16 x3d_rasterregion_edge_x_value(X3D_ScanlineGenerator* gen, int16 y) {
  return gen->a->v2d.x + ((gen->slope.x) * (y - gen->a->v2d.y) >> 16);
}

int16 x3d_t_clip(int16 start, int16 end, uint16 scale) {
  return start + ((((int32)end - start) * scale) >> 15);
}

//_Bool x3d_scanline_generator_set_edge_vex2d(X3D_ScanlineGenerator)






void x3d_rasterregion_draw_outline(X3D_RasterRegion* region, X3D_Color c) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_Span* span = region->span + i - region->rect.y_range.min;
    x3d_screen_draw_pix(span->left.x, i, c);
    x3d_screen_draw_pix(span->right.x, i, x3d_rgb_to_color(0, 255, 255));
  }
}

enum {
  KEY_WIREFRAME = X3D_KEY_0,
  KEY_W = X3D_KEY_1,
  KEY_S = X3D_KEY_2,
  KEY_A = X3D_KEY_3,
  KEY_D = X3D_KEY_4,
  TEST_KEY_ESCAPE = X3D_KEY_5,
  KEY_Q = X3D_KEY_6,
  KEY_E = X3D_KEY_7,
  KEY_UP = X3D_KEY_8,
  KEY_DOWN = X3D_KEY_9,
  KEY_1 = X3D_KEY_10,
  KEY_2 = X3D_KEY_11,
  KEY_9 = X3D_KEY_12,
  KEY_0 = X3D_KEY_13,
  KEY_RECORD = X3D_KEY_14
};

_Bool key_pressed(uint16 key) {
  if(x3d_key_down(key)) {
    while(x3d_key_down(key)) x3d_read_keys();
    
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

uint16 get_polygon(X3D_PolyVertex* v) {
  int16 cx = 640 / 2;
  int16 cy = 480 / 2;
  uint16 total_v = 0;
  
  do {
    x3d_read_keys();
    x3d_screen_draw_pix(cx, cy, 0x7FFF);
    x3d_screen_flip();
    
    if(key_pressed(KEY_WIREFRAME)) {
      v[total_v].v2d.x = cx;
      v[total_v].v2d.y = cy;
      
      ++total_v;
      
      if(total_v >= 2) {
        x3d_screen_draw_line(v[total_v - 1].v2d.x, v[total_v - 1].v2d.y, v[total_v - 2].v2d.x, v[total_v - 2].v2d.y, 31);
        x3d_screen_flip();
      }
    }
    
    if(x3d_key_down(KEY_W))  --cy;
    if(x3d_key_down(KEY_S))  ++cy;
    if(x3d_key_down(KEY_A))  --cx;
    if(x3d_key_down(KEY_D))  ++cx;
    
    SDL_Delay(10);
    
    if(key_pressed(TEST_KEY_ESCAPE))
      break;
  } while(1);
  
  return total_v;
}

extern int16 render_mode;

void x3d_rasterregion_draw(X3D_Vex2D* v, uint16 total_v, X3D_Color c, X3D_RasterRegion* parent, int16 z, X3D_Vex3D* normal, X3D_Vex3D* v3d) {
  X3D_PolyVertex pv[total_v];
  
#if 1
  
  X3D_Polygon2D poly = {
    .v = v,
    .total_v = total_v
  };
  
  v[2].y = v[1].y;
  
  x3d_polygon2d_remove_duplicate(&poly);
  
  uint16 i;
  for(i = 0; i < poly.total_v; ++i) {
    pv[i].v2d = v[i];
    pv[i].intensity = 0x7FFF / (i + 1);
    pv[i].z = 0x7FFF / v3d[i].z;
    
    float a = normal[i].x / 32768.0 / 2 + .5;  //x3d_asin(normal[i].x) / 128.0 + .5;
    float b = normal[i].y / 32768.0 / 2 + .5;//x3d_asin(normal[i].y) / 128.0 + .5;
    
    pv[i].u = a * 191;
    pv[i].v = b * 191;
    
    //pv[i].u = 191 * ((((int32)x3d_asin(normal[i].x) << 15) / 128) + 16384) / 32768;
    //pv[i].v = 191 * ((((int32)x3d_asin(normal[i].y) << 15) / 128) + 16384) / 32768;
  }
  
#if 1
  pv[0].u = 0;
  pv[0].v = 0;
  
  pv[1].u = 0x7FFF / 2;
  pv[1].v = 0x7FFF / 2;
  
  pv[2].u = 0x7FFF;
  pv[2].v = 0x7FFF;
  
  //pv[1].u = 0;
  //pv[1].v = 0x7FFF;
#endif
  
  
  for(i = 0; i < 4; ++i) {
    if(v3d[i].z != 0) {
      pv[i].u = pv[i].u / v3d[i].z;
      pv[i].v = pv[i].v / v3d[i].z;
    }
  }
  
  X3D_RasterRegion r;
  x3d_rasterregion_update(parent);
  
  if(x3d_rasterregion_make(&r, pv, poly.total_v, parent)) {
    x3d_rasterregion_downgrade(&r);
    //x3d_rasterregion_fill_zbuf(&r, c, z);
    
    x3d_rasterregion_fill_texture(&r, z);
    
    //x3d_rasterregion_draw_outline(&r, x3d_rgb_to_color(255, 0, 255));
  }
#endif
}

void x3d_clipregion_test() {
#if 0
  
  X3D_RasterRegion r;
  
  uint16 total_v = 7;
 
#if 0
  X3D_Vex2D v[] = {
    { 250, 350 },
    { 300, 350 },
    { 400, 300 },
    { 350, 200 },
    { 300, 200 },
    { 250, 200 },
    { 200, 250 }
  };
#else
  X3D_Vex2D v[] = {
    { 300, 300 },
    { 400, 300 },
    { 400, 400 },
    { 300, 400 }
  };
  
  total_v = 4;
#endif
  
  uint16 d;
  for(d = 0; d < total_v / 2; ++d)
    X3D_SWAP(v[d], v[total_v - d - 1]);
  
  x3d_screen_clear(0);
  if(!x3d_rasterregion_construct_from_points(&x3d_rendermanager_get()->stack, &r, v, total_v))
    x3d_assert(0);
  
  X3D_PolyVertex pv[30] = {
    {
      .v2d = { 320, 350 },
    },
    {
      .v2d = { 250, 390 },
    },
    {
      .v2d = { 390, 390 }
    }
  };
  
  
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
  
  x3d_rasterregion_update(&r);
  
  total_v = get_polygon(pv);
  
  for(d  = 0; d < total_v; ++d) {
    pv[d].intensity = (d + 1) * 0x7FFFL / (d + 2);
  }
  
  //pv[0].intensity = 0x7FFF;
  //pv[1].intensity = 2 * 0x7FFF / 3;
  //pv[2].intensity = 0x7FFF / 2;
  
  
  int16 left = 0x7FFF;
  int16 right = -0x7FFF;
  
  uint16 i;
  for(i = r.rect.y_range.min; i <= r.rect.y_range.max; ++i) {
    if(r.span[i -r.rect.y_range.min].left.x < left) {
      left = r.span[i - r.rect.y_range.min].left.x;
      r.extreme_left_y = i;
    }
    else if(r.span[i - r.rect.y_range.min].right.x > right) {
      right = r.span[i - r.rect.y_range.min].right.x;
      r.extreme_right_y = i;
    }
  }
  
  x3d_screen_clear(0);
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
  
  
  
  
  //x3d_screen_draw_line(0, min_y, 639, min_y, 0x7FFF);
  //x3d_screen_draw_line(0, max_y, 639, max_y, 0x7FFF);
  
  //x3d_screen_draw_line(left, 0, left, 479, 0x7FFF);
  
  
  
  X3D_RasterRegion r2;
  x3d_rasterregion_make(&r2, pv, total_v, &r);
  
  x3d_rasterregion_draw_outline(&r2, x3d_rgb_to_color(255, 0, 255));

  x3d_rasterregion_downgrade(&r2);
  
  render_mode = 1;
  
  x3d_rasterregion_fill(&r2, x3d_rgb_to_color(0, 0, 255));
  
  x3d_screen_flip();
  
  X3D_RasterRegion dest;
  
  
  do {
    x3d_read_keys();
    
    if(x3d_key_down(X3D_KEY_15)) {
      while(x3d_key_down(X3D_KEY_15)) x3d_read_keys();
      break;
    }
  } while(1);
  
#endif
}









