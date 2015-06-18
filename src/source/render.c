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
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_trig.h"

#ifdef __TIGCC__
#include <extgraph/extgraph.h>
#endif

/// @todo document
void x3d_draw_line_black(X3D_RenderContext* context, X3D_Vex2D_int16 v1, X3D_Vex2D_int16 v2) {
#ifdef __TIGCC__
  //DrawLine(v1.x, v1.y, v2.x, v2.y, A_NORMAL);
  FastLine_Draw_R(context->screen, v1.x, v1.y, v2.x, v2.y);
#endif
}

/// @todo rename context_x and context_y to pos_x and pos_y
void x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags) {

  X3D_STACK_TRACE;

  context->screen = screen;
  context->screen_w = screen_w;
  context->screen_h = screen_h;

  context->w = context_w;
  context->h = context_h;
  context->pos.x = context_x;
  context->pos.y = context_y;

  context->fov = fov;
  context->flags = flags;

  // Default center of the render context
  context->center.x = context->pos.x + context->w / 2;
  context->center.y = context->pos.y + context->h / 2;

  // Calculate the screen scaling factor (distance to the near plane)
  // dist = (w / 2) / tan(fov / 2)
  context->scale = div_int16_by_fp0x16(screen_w / 2, x3d_tanfp(fov / 2));

  //printf("Scale: %d\n", context->scale);
  //ngetchx();
}

/// @todo document
/// @todo make cross platform
void x3d_renderdevice_init(X3D_RenderDevice* d, uint16 w, uint16 h) {
#ifdef __TIGCC__
  d->dbuf = malloc(LCD_SIZE);   /// @todo replace with new memory management
  PortSet(d->dbuf, LCD_WIDTH - 1, LCD_HEIGHT - 1);
#endif
}

/// @todo document
void x3d_renderdevice_cleanup(X3D_RenderDevice* d) {
#ifdef __TIGCC__
  free(d->dbuf);
  PortRestore();
#endif
}

/// @todo document
void x3d_renderdevice_flip(X3D_RenderDevice* d) {
#ifdef __TIGCC__
  FastCopyScreen_R(d->dbuf, LCD_MEM);
#endif
}

/// @todo document
void x3d_rendercontext_clamp_vex2d_int16(X3D_Vex2D_int16* v, X3D_RenderContext* context) {
  if(v->x < context->pos.x)
    v->x = context->pos.x;
  else if(v->x >= context->pos.x + context->w)
    v->x = context->pos.x + context->w - 1;

  if(v->y < context->pos.y)
    v->y = context->pos.y;
  else if(v->y >= context->pos.y + context->h)
    v->y = context->pos.y + context->h - 1;
}

