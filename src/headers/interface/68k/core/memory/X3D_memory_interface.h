#pragma once

#include <stdint.h>
#include <stdio.h>

#include "common/X3D_interface.h"

X3D_INTERFACE static inline void x3d_memory_free(void* ptr) {
  libx3d_memory_free(ptr);
}

X3D_INTERFACE static inline void x3d_memory_free_id(uint16_t id) {
  libx3d_memory_free_id(id);
}


X3D_INTERFACE static inline void x3d_memory_realloc(void* ptr, size_t new_size) {
  libx3d_memory_realloc(ptr, new_size);
}


X3D_INTERFACE static inline void x3d_memory_free_all() {
  libx3d_memory_free_all();
}