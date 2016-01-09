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
#include "X3D_keys.h"
#include "X3D_gameloop.h"

void x3d_game_loop_quit(void) {
  x3d_enginestate_get()->exit_gameloop = X3D_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/// The main game loop.
///////////////////////////////////////////////////////////////////////////////
void x3d_game_loop() {
  X3D_EngineState* state = x3d_enginestate_get();
  
  do {
    // Update the key state
    x3d_read_keys();
    x3d_keymanager_get()->key_handler();
  } while(!state->exit_gameloop);
}

