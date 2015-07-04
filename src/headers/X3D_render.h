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

#pragma once

#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_matrix.h"

//=============================================================================
// Structures
//=============================================================================

/// @todo document
/// @todo rename to X3D_RenderStack
typedef struct {
  uint16 size;
  uint8* ptr;
  uint8* base;
} X3D_Stack;

/// @todo document
typedef struct X3D_List_uint16 {
  uint16* base;
  uint16 size;
  uint16 capacity;
} X3D_List_uint16;

/// @todo document
typedef struct X3D_SegmentTable {
  X3D_List_uint16 segment_offset;
  X3D_Stack segment_data;
} X3D_SegmentTable;

/// A moveable camera
typedef struct {
  X3D_Mat3x3_fp0x16 mat;          ///< Rotation matrix based on angle
  X3D_Vex3D_angle256 angle;       ///< Angle the camera is facing

  X3D_Vex3D_fp16x16 pos;          ///< Position of the camera
  X3D_Vex3D_fp16x16 velocity;     ///< Velocity
} X3D_Camera;

typedef struct X3D_SelectSpinner {
  uint16 selected_segment;
  uint16 selected_face;

  uint16 select_a;
  uint16 select_b;

  uint16 select_data[4];
  uint16 base_v;
} X3D_SelectSpinner;

/// Holds global information for the engine.
typedef struct X3D_EngineState {
  uint16 frame;             ///< Current frame the engine is on
  uint16 render_step;       ///< Which step the renderer is on

  X3D_SegmentTable table;

  X3D_SelectSpinner spinner;
} X3D_EngineState;

/// A logical screen that is rendered to.
typedef struct X3D_RenderContext {
  uint8 flags;                ///< Flags (currently unused)

  /// @todo (should this be moved to camera ? )
  uint8 fov;                  ///< Camera field of view
  X3D_Vex2D_int16 pos;        ///< Position on the screen where to render
  uint16 w, h;                ///< Size of the render context

  int16 scale;                ///< Scaling factor for rendering

  X3D_Vex2D_int16 center;     ///< Logical center of the rendering context

  uint8* screen;              ///< Screen to render to (monochrome)
  uint16 screen_w;            ///< Full witdth of the screen
  uint16 screen_h;            ///< Full height of the screen

  X3D_Stack stack;            ///< Rendering stack

  /// @todo (should this be a 1-1 relationship?)
  X3D_Camera cam;             ///< Camera

  X3D_EngineState* state;

  uint16 render_clock;

} X3D_RenderContext;

/// A physical device to render to
typedef struct X3D_RenderDevice {
  uint8* dbuf;    ///< Double buffer
} X3D_RenderDevice;

struct X3D_Segment;
struct X3D_Frustum;

//=============================================================================
// Function declarations
//=============================================================================

void x3d_draw_line_black(X3D_RenderContext* context, X3D_Vex2D_int16* v1, X3D_Vex2D_int16* v2);

void x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags);

void x3d_renderdevice_init(X3D_RenderDevice* d, uint16 w, uint16 h);
void x3d_renderdevice_cleanup(X3D_RenderDevice* d);
void x3d_renderdevice_flip(X3D_RenderDevice* d);

void x3d_rendercontext_clamp_vex2d_int16(X3D_Vex2D_int16* v, X3D_RenderContext* context);

void x3d_enginestate_init(X3D_EngineState* state, uint16 max_segments, uint16 seg_pool_size);
void x3d_enginestate_cleanup(X3D_EngineState* state);

void x3d_render_segment_wireframe(uint16 id, struct X3D_Frustum* frustum, X3D_EngineState* state, X3D_RenderContext* context);

uint16 x3d_get_clock();

void x3d_selectspinner_select(X3D_SelectSpinner* spinner, X3D_EngineState* state, uint16 segment, uint16 face);

//=============================================================================
// Static inline functions
//=============================================================================

/// @todo document
static inline void* x3d_stack_alloc(X3D_Stack* stack, uint16 bytes) {
  stack->ptr -= (bytes + (bytes & 1));      // Guarantees we allocate an even number of bytes
  return stack->ptr;
}

/// @todo document
static inline void* x3d_stack_save(X3D_Stack* stack) {
  return stack->ptr;
}

/// @todo document
static inline void x3d_stack_restore(X3D_Stack* stack, void* ptr) {
  stack->ptr = ptr;
}

/// @todo document
static inline void x3d_stack_create(X3D_Stack* stack, uint16 size) {
  /// @TODO: replace with x3d_malloc
  stack->base = malloc(size);
  stack->size = size;
  stack->ptr = stack->base + size;
}

/// @todo document
/// @todo check if capacity is exceeded
static inline uint16 x3d_list_uint16_add(X3D_List_uint16* list, uint16 value) {
  X3D_STACK_TRACE;

  list->base[list->size] = value;
  return list->size++;
}

/// @todo document
static inline void x3d_list_uint16_create(X3D_List_uint16* list, uint16 size) {
  list->base = malloc(size);
  list->size = 0;
  list->capacity = size;
}

static inline void x3d_get_selectspinner_selected(X3D_SelectSpinner* spinner, uint16* a, uint16* b) {
  if(spinner->selected_face == 0) {
    *a = spinner->select_a;
    *b = spinner->select_b;
  }
  else if(spinner->selected_face == 1) {
    *a = spinner->select_a + spinner->base_v;
    *b = spinner->select_b + spinner->base_v;
  }
  else {
    *a = spinner->select_data[spinner->select_a];
    *b = spinner->select_data[spinner->select_b];
  }
}

static inline uint16 x3d_single_wrap(uint16 v, uint16 max) {
  if(v >= max) {
    return v - max;
  }

  return v;
}

static inline void x3d_selectspinner_spin(X3D_SelectSpinner* spinner) {
  spinner->select_a = x3d_single_wrap(spinner->select_a + 1, spinner->base_v);
  spinner->select_b = x3d_single_wrap(spinner->select_b + 1, spinner->base_v);
}

