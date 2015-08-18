/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "resource/X3D_memory.h"

#define ALLOCA_POLYGON3D(_total_v) alloca(x3d_polygon3d_needed_size(_total_v))
#define ALLOCA_FRUSTUM(_total_p) alloca(x3d_frustum_needed_size(_total_p))

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
  stack->base = x3d_malloc(NULL, size);
  stack->size = size;
  stack->ptr = stack->base + size;
}

/// @todo document
/// @todo check if capacity is exceeded
static inline uint16 x3d_list_uint16_add(X3D_List_uint16* list, uint16 value) {

  list->base[list->size] = value;
  return list->size++;
}

/// @todo document
static inline void x3d_list_uint16_create(X3D_List_uint16* list, uint16 size) {
  list->base = x3d_malloc(NULL, size);
  list->size = 0;
  list->capacity = size;
}