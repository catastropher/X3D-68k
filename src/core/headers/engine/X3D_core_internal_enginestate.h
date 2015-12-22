// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.
//
//
//
// X3D_enginestate.h -> functionality for manipulating the global engine state.

#pragma once

#include "memory/X3D_core_memory.h"

///////////////////////////////////////////////////////////////////////////////
/// Stores the global state for the entire engine.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL typedef struct X3D_EngineState {
  X3D_MemoryManager memory_manager;     ///< Memory manager data
} X3D_EngineState;

// The declaration of x3d_engine_state (see X3D_config.h for the current
//    platform to see the exact definition). 
X3D_ENGINE_STATE_DECL;

///////////////////////////////////////////////////////////////////////////////
/// Returns a pointer to the engine's memory manager data.
///
/// @return A pointer to the engine's memory manager data.
/// @note For internal use only.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL static inline X3D_MemoryManager* x3d_memory_manager_data() {
  return &x3d_engine_state->memory_manager;
}