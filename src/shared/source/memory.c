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
 
#define X3D_MAX_ALLOCS 10

#include "X3D_fix.h"
#include "X3D_error.h"
#include "X3D_memory.h"

#include "../headers/X3D_fix.h"

typedef struct X3D_Alloc {
  void* mem;
  uint16 size;
  char* function_name;
} X3D_Alloc;

typedef struct X3D_MemoryManager {
  X3D_Alloc allocs[X3D_MAX_ALLOCS];
} X3D_MemoryManager;

X3D_MemoryManager mem_manager;

void* x3d_malloc_function(uint16 size, const char* function_name) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem_manager.allocs[i].mem == NULL) {
      mem_manager.allocs[i].mem = malloc(size);
      
      if(mem_manager.allocs[i].mem == NULL) {
        x3d_error("Out of mem (needed %u)", size); 
      }
      
      mem_manager.allocs[i].size = size;
      mem_manager.allocs[i].function_name = (char *)function_name;
      
      return mem_manager.allocs[i].mem;
    }
  }
  
  x3d_error("Too many allocs");
}

void x3d_free_function(void* mem, const char* function_name) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem_manager.allocs[i].mem == mem) {
      free(mem_manager.allocs[i].mem); 
      mem_manager.allocs[i].mem = NULL;
      return;
    }
  }
  
  x3d_error("Bad free in %s", function_name);
}

void x3d_free_all() {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem_manager.allocs[i].mem != NULL) {
      free(mem_manager.allocs[i].mem);
    }
  }
}

void x3d_init_memorymanager() {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    mem_manager.allocs[i].mem = NULL;
  }  
  
}

/***
 * Allocates a new memory block from a fixed-size block allocator. 
 * 
 * @param ba  - the block allocator to allocate from
 * 
 * @note If out of the blocks, this will currently throw an error. In future,
 *    it will allocate more memory from the free store.
 */
void* x3d_alloc_block(X3D_BlockAllocator* ba) {
  if(ba->head == NULL) {
    // TODO: allocate more memory from the free store
    x3d_error("Block allocator full");
  }
  
  X3D_BlockAllocatorNode* node = ba->head;
  ba->head = ba->head->next;
  return (void *)node - ba->pointer_offset;
}

/***
 * Returns a memory block to a block allocator
 * 
 * @param ba    - block allocator
 * @param block - block to free
 * 
 * @note Only free blocks to the block allocator it was allocated from!
 */
void x3d_free_block(X3D_BlockAllocator* ba, void* block) {
  X3D_BlockAllocatorNode* node = (X3D_BlockAllocatorNode* )(block + ba->pointer_offset);
  
  if(ba->tail == NULL) {
    ba->tail = ba->head = node;
  }
  else {
    node->next = ba->tail;
    ba->tail = node;
  }
}

/***
 * Initializes a block allocator, which allocates single blocks of a fixed size.
 * 
 * @param ba              - block allocator
 * @param block_size      - size of each block
 * @param pointer_offset  - a word-aligned offset in a block that's safe to
 *                            over-write when the block is not in use (used
 *                            to store a pointer to the next block)
 * @param memory          - memory to partition into blocks
 * @param memory_size     - size of the memory to partition
 */
void x3d_init_blockallocator(X3D_BlockAllocator* ba, uint16 block_size,
            uint16 pointer_offset, void* memory, uint16 memory_size) {

    ba->head = ba->tail = NULL;
    ba->block_size = block_size;
    ba->pointer_offset = pointer_offset;
    
    void* ptr = memory + memory_size - block_size;
    
    do {
      x3d_free_block(ba, ptr);
      ptr -= block_size;
    } while(ptr >= memory);
}















