#include "X3D_config.h"
#include "X3D_fix.h"

typedef struct {
  int16 size;
  uint8* ptr;
  uint8* base;
} X3D_Stack;

typedef struct X3D_RenderContext {
  uint8 flags;
  uint8 fov;
  int16 x, y;
  int16 w, h;
  
} X3D_RenderContext;


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
