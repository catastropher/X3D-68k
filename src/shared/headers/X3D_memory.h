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

#include "X3D_fix.h"

//=============================================================================
// Macros
//=============================================================================

#define x3d_malloc(_size)   x3d_malloc_function(_size, __FUNCTION__)
#define x3d_free(_mem)      x3d_free_function(_mem, __FUNCTION__)

//=============================================================================
// Types
//=============================================================================

typedef struct X3D_BlockAllocatorNode {
  struct X3D_BlockAllocatorNode* next;
} X3D_BlockAllocatorNode;

typedef struct X3D_BlockAllocator {
  void* memory_base;
  void* first_block;
  X3D_BlockAllocatorNode* head;
  X3D_BlockAllocatorNode* tail;
  uint16 block_size;
  uint16 pointer_offset;
} X3D_BlockAllocator;

//=============================================================================
// Function declarations
//=============================================================================

void* x3d_malloc_function(uint16 size, const char* function_name);
void x3d_free_function(void* mem, const char* function_name);
void x3d_free_all();
void x3d_init_memorymanager();

void* x3d_alloc_block(X3D_BlockAllocator* ba);
void x3d_free_block(X3D_BlockAllocator* ba, void* block);
void x3d_init_blockallocator(X3D_BlockAllocator* ba, uint16 block_size,
                             uint16 pointer_offset, void* memory, uint16 memory_size);
void* x3d_get_block(X3D_BlockAllocator* ba, uint16 id);

