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
#include "memory/X3D_varsizeallocator.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes a variable-sized object allocator.
///
/// @param alloc      - variable-size object allocator
/// @param max_allocs - maximum number of allocations that can be made
/// @param mem_size   - size of memory pool for allocating objects
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_varsizeallocator_init(X3D_VarSizeAllocator* alloc, uint16 max_allocs, uint16 mem_size) {
  uint32 total_size = (uint32)max_allocs * sizeof(uint16) + (uint32)mem_size;
  
  // Max size of allocation in 68k (we do this as one allocation to save space)
  x3d_assert(total_size < 60000);
  
  /// @todo Replace with x3d memory allocation routine
  void* mem = malloc(total_size);
  x3d_assert(mem);
  
  x3d_list_uint16_init(&alloc->alloc_offset, mem, max_allocs * sizeof(uint16));
  x3d_stack_init(&alloc->alloc_pool, mem + max_allocs * sizeof(uint16), mem_size);
}

///////////////////////////////////////////////////////////////////////////////
/// Allocates a variable-sized object from a variable-sized allocator.
///
/// @param alloc  - variable-sized allocator
/// @param size   - size of the object to allocate
/// @param id     - where to write the id of the object being allocated
///
/// @return The address of the allocated object
///////////////////////////////////////////////////////////////////////////////
void* x3d_varsizeallocator_alloc(X3D_VarSizeAllocator* alloc, uint16 size, uint16* id) {
  void* obj = x3d_stack_alloc(&alloc->alloc_pool, size);
  
  /// @todo We shouldn't access the stack's base memory directly
  *id = x3d_list_uint16_add(&alloc->alloc_offset, obj - alloc->alloc_pool.base);
  
  return obj;
}

void* x3d_varsizeallocator_get(X3D_VarSizeAllocator* alloc, uint16 id) {
  
}

