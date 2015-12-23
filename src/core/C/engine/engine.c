#include "X3D_core.h"

#include "engine/X3D_core_internal_enginestate.h"

X3D_EngineState x3d_global_state;

#ifdef X3D_CUSTOM_ENGINE_STATE

X3D_ENGINE_STATE_DEFINITION;

#else

X3D_EngineState* x3d_engine_state;

#endif