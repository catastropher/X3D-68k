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
#include "memory/X3D_stack.h"
#include "X3D_assert.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes an X3D_Stack.
///
/// @param stack    - stack
/// @param mem      - memory for the stack
/// @param mem_size - size of the memory for the stack
///////////////////////////////////////////////////////////////////////////////
void x3d_stack_init(X3D_Stack* stack, void* mem, uint32 mem_size) {
  stack->base = mem;
  stack->size = mem_size;
  
  // The stack starts at the end of memory and is decremented before each
  // alloc
  stack->ptr = mem + mem_size;
}

///////////////////////////////////////////////////////////////////////////////
/// Allocates space on a stack.
///
/// @param stack  - stack
/// @param size   - size of chunk to allocate
///
/// @return A pointer to the chunk of memory that is at least enough to hold
///   the requested size.
/// @note The chunk may be bigger by a byte because 68k requires memory to be
///   aligned to a 2-byte boundry.
///
/// @todo Make this safe for devices that require 4-byte memory alignment.
///////////////////////////////////////////////////////////////////////////////
void* x3d_stack_alloc(X3D_Stack* stack, uint16 size) {
  // Allocate space that is guaranteed to have an even size
  // (68k must have memory aligned to an even boundry)
  stack->ptr -= size;
  stack->ptr -= ((size_t)stack->ptr) & (X3D_WORD_ALIGN - 1);
  
  // Check for stack overflow
  x3d_assert(stack->ptr >= stack->base);
  
  return stack->ptr;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a stack's current stack pointer, which can be restored with @ref
///   x3d_stack_restore(). This is useful for freeing several stack allocation
///   at once by restoring the stack to its position before the allocations
///   occured.
///
/// @param stack  - stack
///
/// @return The stack's current stack pointer.
///////////////////////////////////////////////////////////////////////////////
void* x3d_stack_save(X3D_Stack* stack) {
  return stack->ptr;
}

///////////////////////////////////////////////////////////////////////////////
/// Sets a stack's stack pointer.
///
/// @param stack  - stack
/// @param ptr    - pointer to set the stack's stack pointer to
///
/// @note See @ref x3d_stack_save().
///////////////////////////////////////////////////////////////////////////////
void x3d_stack_restore(X3D_Stack* stack, void* ptr) {
  x3d_assert(ptr > stack->base && ptr <= stack->base + stack->size);
  
  stack->ptr = ptr;
}

///////////////////////////////////////////////////////////////////////////////
/// Resets the stack to its original stack pointer
///
/// @param stack  - stack
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_stack_reset(X3D_Stack* stack) {
  stack->ptr = stack->base + stack->size;
}


