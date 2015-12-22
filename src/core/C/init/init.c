#include "X3D.h"

#include "engine/X3D_core_internal_enginestate.h"

extern X3D_EngineState x3d_global_state;

X3D_IMPLEMENTATION void x3dcore_init() {
  x3d_engine_state = &x3d_global_state;
}