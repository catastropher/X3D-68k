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
#include "memory/X3D_alloc.h"
#include "memory/X3D_slaballocator.h"
#include "X3D_enginestate.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes the alloc manager with the given memory pool size.
///
/// @param mem_size - size of the memory pool to create
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_allocmanager_init(size_t mem_size) {
  x3d_slaballocator_init(&x3d_allocmanager_get()->alloc, mem_size);
}

///////////////////////////////////////////////////////////////////////////////
/// Allocates a small block ( < 512 bytes) from the global engine slab
///   allocator.
///
/// @param size - size of the block to allocate
///
/// @return A block of memory that is at least size bytes.
///////////////////////////////////////////////////////////////////////////////
void* x3d_slab_alloc(uint16 size) {
  return x3d_slaballocator_alloc(&x3d_allocmanager_get()->alloc, size);
}

///////////////////////////////////////////////////////////////////////////////
/// Frees a block back to the global engine slab allocator.
///
/// @param mem  - block to free
///
/// @return Nothing.
/// @note No checks are performed to make sure that mem was actually alloced
///   from alloc - be careful!
///////////////////////////////////////////////////////////////////////////////
void x3d_slab_free(void* mem) {
  x3d_slaballocator_free(&x3d_allocmanager_get()->alloc, mem);
}

