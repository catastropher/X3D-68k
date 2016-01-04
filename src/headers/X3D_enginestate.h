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

typedef struct X3D_EngineState {
  X3D_SegmentManager segment_manager;
  X3D_ScreenManager screen_manager;
} X3D_EngineState;

extern X3D_EngineState* x3d_state;

static inline X3D_SegmentManager* x3d_segmentmanager_get(void) {
  return &x3d_state->segment_manager;
}

static inline const X3D_ScreenManager* x3d_screenmanager_get(void) {
  return &x3d_state->screen_manager;
}

X3D_INTERNAL void x3d_enginestate_init(void);

