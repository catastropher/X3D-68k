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

























#if 0






typedef struct X3D_Hashentry_Vex3D {
  X3D_Vex3D_int16 v;
  uint16 key;
  uint16 frame;
} X3D_Hashentry_Vex3D;

#define X3D_RENDER_HASHTABLE_SIZE 32

typedef struct X3D_Hashtable_Vex3D {
  X3D_Hashentry_Vex3D v[X3D_RENDER_HASHTABLE_SIZE];
} X3D_Hashtable_Vex3D;


static inline void* x3d_stack_alloc(X3D_Stack* stack, uint16 bytes) {
  stack->ptr -= bytes;
  return stack->ptr;
}

void* x3d_stack_save(X3D_Stack* stack) {
  return stack->ptr;
}

static inline void x3d_stack_restore(X3D_Stack* stack, void* ptr) {
  stack->ptr = ptr;
}

static inline void x3d_stack_create(X3D_Stack* stack, uint16 size) {
  /// @TODO: replace with x3d_malloc
  stack->base = malloc(size);
  stack->size = size;
  stack->ptr = stack->base + size;
}

void x3d_segment_render(X3D_Segment* seg, X3D_RenderContext* context) {
  void* save_stack = x3d_stack_save(&context->stack);



  x3d_stack_restore(&context->stack, save_stack);
}

#endif

