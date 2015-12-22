#pragma once

typedef struct X3D_MemoryManagerAlloc {
  uint16 id;
  void* data;
} X3D_MemoryManagerAlloc;

typedef struct X3D_MemoryManager {
  X3D_MemoryManagerAlloc allocs[X3D_MAX_ALLOCS];
} X3D_MemoryManager;