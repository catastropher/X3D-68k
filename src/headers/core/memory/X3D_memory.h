#pragma once

#include "common/X3D_interface.h"

#include <stdint.h>
#include <stdio.h>

//=============================================================================
// 
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
/// Allocates a block of memory.
///
/// @param group_id   - a group id (need not be unique)
/// @param size       - size of the block to allocate
///
/// @note If out of memory, this will throw X3D_OUT_OF_MEM.
///////////////////////////////////////////////////////////////////////////////
X3D_IMPLEMENTATION void* libx3d_memory_alloc(uint16 group_id, size_t size);

///////////////////////////////////////////////////////////////////////////////
/// Free the block of memory pointed to by a pointer.
///
/// @param group_id   - a group id (need not be unique)
///////////////////////////////////////////////////////////////////////////////
X3D_IMPLEMENTATION void libx3d_memory_free(void* ptr);
X3D_IMPLEMENTATION void libx3d_memory_free_id(uint16 id);
X3D_IMPLEMENTATION void libx3d_memory_realloc(void* ptr, size_t new_size);
X3D_IMPLEMENTATION void libx3d_memory_free_all();

X3D_INTERFACE static inline void* x3d_memory_alloc(uint16 group_id, size_t size);
X3D_INTERFACE static inline void x3d_memory_free(void* ptr);
X3D_INTERFACE static inline void x3d_memory_free_id(uint16 id);
X3D_INTERFACE static inline void x3d_memory_realloc(void* ptr, size_t new_size);
X3D_INTERFACE static inline void x3d_memory_free_all();