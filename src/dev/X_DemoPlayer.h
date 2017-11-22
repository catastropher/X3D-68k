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

#include "X_DemoRecorder.h"

typedef struct X_DemoPlayerFrame
{
    unsigned char keyState[X_KEY_BITARRAY_SIZE];
} X_DemoPlayerFrame;

typedef struct X_DemoPlayer
{
    X_CameraObject* cam;
    X_KeyState* keyState;
    
    int totalFrames;
    X_DemoPlayerFrame* frames;
    
    int currentFrame;
} X_DemoPlayer;

void x_demoplayer_init(X_DemoPlayer* player, X_CameraObject* cam, X_KeyState* keyState);
_Bool x_demoplayer_play(X_DemoPlayer* player, const char* fileName);
void x_demoplayer_play_frame(X_DemoPlayer* player);

static inline _Bool x_demoplayer_is_playing(const X_DemoPlayer* player)
{
    return player->frames != NULL && player->currentFrame < player->totalFrames;
}

