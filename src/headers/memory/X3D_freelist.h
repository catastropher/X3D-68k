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

#include "X3D_common.h"

/// Indicates that a freelist doesn't keep track of the block id in the block
#define X3D_FREELIST_NO_BLOCK_ID -1

///////////////////////////////////////////////////////////////////////////////
/// A memory that allocates single blocks of a fixed-size in O(1) time.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_FreeList {
  int16 id;
  void* begin;
  void* end;
  int16 block_size;
  int16 next_pointer_offset;
  
  void* head;
  void* tail;
} X3D_FreeList;

void x3d_freelist_create(X3D_FreeList* list, void* mem, size_t mem_size,
    int16 block_size, int16 next_pointer_offset, int16 block_id_offset,
    int16 id);

void* x3d_freelist_alloc(X3D_FreeList* list);
void* x3d_freelist_free(X3D_FreeList* list, void* block);
void* x3d_freelist_get_block(X3D_FreeList* list, int16 id);

