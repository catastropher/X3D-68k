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

#include "X3D_common.h"
#include "X3D_enginestate.h"
#include "X3D_player.h"
#include "memory/X3D_stack.h"
#include "X3D_assert.h"
#include "X3D_clip.h"
#include "X3D_init.h"
#include "X3D_camera.h"
#include "memory/X3D_handle.h"
#include "memory/X3D_alloc.h"

static X3D_EngineState x3d_global_enginestate;
X3D_EngineState* const x3d_state = &x3d_global_enginestate;

///////////////////////////////////////////////////////////////////////////////
/// A dummy callback.
///////////////////////////////////////////////////////////////////////////////
void x3d_dummy_void_callback(void) {
  
}

///////////////////////////////////////////////////////////////////////////////
/// Initializes the global engine state.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL void x3d_enginestate_init(X3D_InitSettings* init) {
  //x3d_state = &x3d_global_enginestate;
  
  x3d_allocmanager_init(12000);
  x3d_handlemanager_init(100);
  x3d_rendermanager_init(init);
  x3d_objectmanager_init();
  
  x3d_keymanager_get()->key_handler = x3d_dummy_void_callback;
  
  // Reset engine step
  /// @todo Should engine_step be moved into the render manager?
  x3d_state->engine_step = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up the engine state.
///////////////////////////////////////////////////////////////////////////////
void x3d_enginestate_cleanup(void) {
  x3d_handlemanager_cleanup();
  x3d_allocmanager_cleanup();
}

