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

void x3d_free_function(void* mem, void* function_name) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem_manager.allocs[i].mem == mem) {
      free(mem_manager.allocs[i].mem); 
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



