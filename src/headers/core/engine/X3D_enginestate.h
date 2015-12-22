#pragma once

X3D_INTERNAL typedef struct X3D_EngineState {
  X3D_MemoryManager memory_manager;
} X3D_EngineState;

X3D_ENGINE_STATE_DECL;

X3D_INTERNAL static inline X3D_MemoryManager* x3d_memory_manager_data() {
  return &x3d_engine_state->memory_manager;
}