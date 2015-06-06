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

typedef struct {
  int16 size;
  uint8* ptr;
  uint8* base;
} X3D_Stack;

typedef struct X3D_RenderContext {
  uint8 flags;
  uint8 fov;
  uint16 x, y;
  uint16 w, h;

  int16 scale;

  uint16 center_x;
  uint16 center_y;
  
  X3D_Stack stack;    /// Render stack
  
  uint8* screen;
  uint16 screen_w;
  uint16 screen_h;

} X3D_RenderContext;

typedef struct X3D_EngineState {
  uint16 frame;
  uint16 render_delta;
} X3D_EngineState;


void x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w, uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags) {
  context->screen = screen;
  context->screen_w = screen_w;
  context->screen_h = screen_h;

  context->w = context_w;
  context->h = context_h;
  context->x = context_x;
  context_y = context_y;

  context->fov = fov;
  context->flags = flags;

  // Default center of the render context
  context->center_x = context->x + context->w / 2;
  context->center_y = context->y + context->h / 2;

  // Calculate the screen scaling factor (distance to the near plane)
  // dist = (w / 2) / tan(fov / 2)
  //c->dist = FIXDIV8(w / 2, tanfp(fov / 2));
}

































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

