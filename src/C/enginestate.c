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

static X3D_EngineState x3d_global_enginestate;
X3D_EngineState* x3d_state;

///////////////////////////////////////////////////////////////////////////////
/// A dummy callback.
///////////////////////////////////////////////////////////////////////////////
void x3d_dummy_void_callback(void) {
  
}

///////////////////////////////////////////////////////////////////////////////
/// Initializes the global engine state.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL void x3d_enginestate_init(void) {
  x3d_state = &x3d_global_enginestate;
  
  x3d_segmentmanager_init(10, 2000);
  
  x3d_keymanager_get()->key_handler = x3d_dummy_void_callback;
  
  /// @todo Once the object system is created, let the player create their
  ///   own camera instead of statically creating one here!
  static X3D_CameraObject cam;
  x3d_playermanager_get()->player[0] = (X3D_Player) {
    .cam = &cam,
    .id = 0
  };
  
  // Reset engine step
  x3d_state->engine_step = 0;
}

