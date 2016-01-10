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

#pragma once

#include "X3D_common.h"
#include "X3D_segment.h"
#include "X3D_screen.h"
#include "X3D_keys.h"
#include "X3D_player.h"

typedef struct X3D_EngineState {
  X3D_SegmentManager segment_manager;
  X3D_ScreenManager screen_manager;
  X3D_KeyManager key_manager;
  X3D_PlayerManager player_manager;
  
  _Bool exit_gameloop;
  uint16 engine_step;
} X3D_EngineState;

extern X3D_EngineState* x3d_state;

static inline X3D_SegmentManager* x3d_segmentmanager_get(void) {
  return &x3d_state->segment_manager;
}

static inline const X3D_ScreenManager* x3d_screenmanager_get(void) {
  return &x3d_state->screen_manager;
}

static inline X3D_KeyManager* x3d_keymanager_get(void) {
  return &x3d_state->key_manager;
}

static inline X3D_EngineState* x3d_enginestate_get(void) {
  return x3d_state;
}

static inline void x3d_keymanager_set_callback(void (*callback)(void)) {
  x3d_keymanager_get()->key_handler = callback;
}

static inline X3D_PlayerManager* x3d_playermanager_get(void) {
  return &x3d_state->player_manager;
}

static inline uint16 x3d_enginestate_get_step(void) {
  return x3d_state->engine_step;
}

static inline void x3d_enginestate_next_step(void) {
  ++x3d_state->engine_step;
}


X3D_INTERNAL void x3d_enginestate_init(void);

