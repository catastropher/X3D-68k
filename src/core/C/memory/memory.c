#include <stdlib.h>

#include "X3D_core.h"

#include "engine/X3D_core_internal_enginestate.h"

X3D_IMPLEMENTATION void* libx3d_memory_alloc(uint16 group_id, size_t size) {
  X3D_MemoryManager* mem = x3d_memory_manager_data();
  uint16 i;

  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem->allocs[i].data == NULL) {
      /// @todo replace with platform malloc
      mem->allocs[i].data = malloc(size);
      mem->allocs[i].id = group_id;

      return mem->allocs[i].data;
    }
  }

  /// @todo throw error!
}


X3D_IMPLEMENTATION void libx3d_memory_free(void* ptr) {
  X3D_MemoryManager* mem = x3d_memory_manager_data();
  uint16 i;

  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem->allocs[i].data == NULL) {
      /// @todo replace with platform free
      free(mem->allocs[i].data);
      mem->allocs[i].data = NULL;
      return;
    }
  }
}

X3D_IMPLEMENTATION void libx3d_memory_free_id(uint16 id) {
  X3D_MemoryManager* mem = x3d_memory_manager_data();
  uint16 i;

  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem->allocs[i].id == id) {
      /// @todo replace with platform free
      free(mem->allocs[i].data);
      mem->allocs[i].data = NULL;
    }
  }
}

X3D_IMPLEMENTATION void libx3d_memory_realloc(void* ptr, size_t new_size) {
  X3D_MemoryManager* mem = x3d_memory_manager_data();
  uint16 i;

  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem->allocs[i].data == ptr) {
      /// @todo replace with platform realloc
      mem->allocs[i].data = realloc(mem->allocs[i].data, new_size);
      return;
    }
  }
}

X3D_IMPLEMENTATION void libx3d_memory_free_all() {
  X3D_MemoryManager* mem = x3d_memory_manager_data();
  uint16 i;

  for(i = 0; i < X3D_MAX_ALLOCS; ++i) {
    if(mem->allocs[i].data != NULL) {
      /// @todo replace with platform free
      free(mem->allocs[i].data);
      mem->allocs[i].data = NULL;
    }
  }
}
