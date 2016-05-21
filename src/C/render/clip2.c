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
#include "render/X3D_rasterregion.h"

//#define //x3d_log(...) ;


///////////////////////////////////////////////////////////////////////////////
/// Draws a polyline (for debugging).
///
/// @param p - polyline
/// @param c - color
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polyline_draw(X3D_PolyLine* p, X3D_Color c) {
  uint16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
    uint16 next = (i + 1 < p->total_v ? i + 1 : 0);
    
    x3d_screen_draw_line(p->v[i]->v2d.x, p->v[i]->v2d.y, p->v[next]->v2d.x, p->v[next]->v2d.y, c);
  }
  
  
  
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the change in d_a with respect to d_b (the slope).
///
/// @param d_a  - e.g. change in x
/// @param d_b  - e.g. change in y
///
/// @return Rate of change as an fp16x16
///////////////////////////////////////////////////////////////////////////////
fp16x16 x3d_val_slope(int16 d_a, int16 d_b) {
  if(d_b == 0) return 0;
  return ((int32)d_a << 16) / d_b;
}

fp16x16 x3d_val_slope2(fp16x16 d_a, int16 d_b) {
  if(d_b == 0) return 0;
  return d_a / d_b;
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates how far along a line segment a point is.
///
/// @param a  - starting point of the line segment
/// @param b  - ending point of the line segment
/// @param v  - some point on the line segment
///
/// @return How far v is along ab (between 0 and 1 as an fp0x16).
///////////////////////////////////////////////////////////////////////////////
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
    
    x3d_rendermanager_get()->zbuf[i * x3d_screenmanager_get()->h + span->left.x] = 0x7FFF;
    x3d_rendermanager_get()->zbuf[i * x3d_screenmanager_get()->h + span->right.x] = 0x7FFF;
  }
}

enum {
  KEY_ENTER = X3D_KEY_0,
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

void x3d_rasterregion_draw(X3D_Vex2D* v, uint16 total_v, X3D_RasterRegion* parent, X3D_Vex3D* v3d, X3D_PolygonAttributes* att) {
  X3D_PolyVertex pv[total_v];
  
  X3D_Polygon2D poly = {
    .v = v,
    .total_v = total_v
  };
  
  x3d_polygon2d_remove_duplicate(&poly);

  uint16 i;
  for(i = 0; i < poly.total_v; ++i) {
    pv[i].v2d = v[i];
    pv[i].intensity = 0;//0x7FFF / (i + 1);
    
    pv[i].z = (1L << 30) / (v3d[i].z != 0 ? v3d[i].z : 1);
   
    if(att->flags & X3D_POLYGON_TEXTURE) {
      pv[i].u = ((int32)att->texture.uu[i] << 22) / v3d[i].z;
      pv[i].v = ((int32)att->texture.vv[i] << 22) / v3d[i].z;
    }
  }
  
  X3D_RasterRegion r;
  x3d_rasterregion_update(parent);
  x3d_rasterregion_make(&r, pv, poly.total_v, parent, att->flags & X3D_POLYGON_TEXTURE);
  
  if(att->flags & X3D_POLYGON_COLOR) {
    x3d_rasterregion_draw_color(&r, att->color);
  }
}

void x3d_rasterregion_draw_color(X3D_RasterRegion* region, X3D_Color color) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_Span2 new_span;
    
    X3D_Span* span = x3d_rasterregion_get_span(region, i);
    
    new_span.left.x = span->left.x;
    new_span.left.z = 0x7FFF << 7;
    
    new_span.right.x = span->right.x;
    new_span.right.z = 0x7FFF << 7;
    
    x3d_screen_draw_scanline_color(&new_span, i, color);
  }
}

