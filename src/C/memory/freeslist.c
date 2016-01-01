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

#include <stdio.h>

#include "X3D_common.h"
#include "memory/X3D_freelist.h"
#include "X3D_assert.h"

///////////////////////////////////////////////////////////////////////////////
/// Creates a new freelist for allocating fixed-sized blocks.
///
/// @param list                 - freelist
/// @param mem                  - memory to partition into blocks
/// @param mem_size             - size of the memory to partition
/// @param block_size           - size of blocks in the freelist
/// @param next_pointer_offset  - an offset from the beginning of each block
///     that is safe to overwrite with a pointer.
/// @param block_id_offset      - an offset from the beginning of each block
///     where the id of the block should be written (as a uint16). This is
///     optional (pass X3D_FREELIST_NO_BLOCK_ID to disable).
/// @param id                   - unique id of the freelist (for debugging)
///
/// @note mem_size MUST be a multiple of block_size!
/// @note The blocks are created and added to the freelist in order, but when
///     allocated will be allocated in reverse order!
///////////////////////////////////////////////////////////////////////////////
void x3d_freelist_create(X3D_FreeList* list, void* mem, size_t mem_size,
    int16 block_size, int16 next_pointer_offset, int16 block_id_offset,
    int16 id) {

  // Only allow sizes that are the multiple of the block size
  x3d_assert((mem_size % block_size) == 0);
  
  list->begin = mem;
  list->end = list->begin + mem_size;
  list->block_size = block_size;
  list->next_pointer_offset = next_pointer_offset;
  list->id = id;
  
  int16 block_id = 0;
  void* block = list->begin - list->block_size;
  void** next_ptr;
  
  do {
    block += list->block_size;
    
    // Write in the id of the current block, if enabled
    if(block_id_offset != X3D_FREELIST_NO_BLOCK_ID) {
      uint16* id_ptr = (uint16 *)(block + block_id_offset);  
      *id_ptr = block_id++;
    }

    // Pointer to the pointer to the next block
    next_ptr = (void **)block + list->next_pointer_offset;
    *next_ptr = block + list->block_size;
  } while(block < list->end);
  
  // Last block doesn't have a next block...
  *next_ptr = NULL;
  
  list->head = list->begin;
  list->tail = block;
  
  x3d_log(X3D_INFO, "Created freelist %d with %d blocks (block size=%d bytes, "
    "id_offset=%d, next_ptr_offset=%d", id, block_id, block_size,
    block_id_offset, next_pointer_offset);
}

void* x3d_freelist_alloc(X3D_FreeList* list) {
  if(list->head) {
    void* block = list->head;
    void** next = (void **)block + list->next_pointer_offset;
    
    list->head = *next;
    return block;
  }
  
  /// @todo throw an error because we're out of blocks!!!
}

