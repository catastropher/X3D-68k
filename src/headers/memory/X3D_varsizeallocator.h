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

#pragma once

#include <stdlib.h>

#include "X3D_common.h"
#include "memory/X3D_list.h"
#include "memory/X3D_stack.h"
#include "X3D_assert.h"

///////////////////////////////////////////////////////////////////////////////
/// A memory allocator for variable-size objects.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_VarSizeAllocator {
  X3D_Stack alloc_pool;             // Memory available to the allocator
  X3D_List_uint16 alloc_offset;     // List of allocation offsets
} X3D_VarSizeAllocator;

void x3d_varsizeallocator_init(X3D_VarSizeAllocator* alloc, uint16 max_allocs, uint16 mem_size);
void* x3d_varsizeallocator_alloc(X3D_VarSizeAllocator* alloc, uint16 size, uint16* id);
void* x3d_varsizeallocator_get(X3D_VarSizeAllocator* alloc, uint16 id);

