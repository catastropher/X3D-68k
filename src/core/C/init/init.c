#include "X3D.h"

#include "X3D_config.h"

#include "engine/X3D_core_internal_enginestate.h"

extern X3D_EngineState x3d_global_state;

X3D_IMPLEMENTATION _Bool x3dcore_init() {
  x3d_engine_state = &x3d_global_state;
  
  return X3D_TRUE;
}