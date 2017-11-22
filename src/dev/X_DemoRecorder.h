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

#include "system/X_File.h"
#include "object/X_CameraObject.h"
#include "system/X_Keys.h"

typedef struct X_DemoRecorder
{
    X_File file;
    X_CameraObject* cam;
    X_KeyState* keyState;
    int totalFrames;
} X_DemoRecorder;

_Bool x_demorecorder_init(X_DemoRecorder* recorder, X_CameraObject* cam, X_KeyState* keyState, const char* outputFileName);
void x_demorecorder_cleanup(X_DemoRecorder* recorder);

void x_demorecorder_save_frame(X_DemoRecorder* recorder);

