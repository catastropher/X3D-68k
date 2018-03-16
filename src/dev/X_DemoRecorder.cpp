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

#include "X_DemoRecorder.h"
#include "error/X_log.h"
#include "engine/X_EngineContext.h"

static void save_camera_state(X_DemoRecorder* recorder, X_CameraObject* cam)
{
    x_file_write_mat4x4(&recorder->file, &cam->viewMatrix);

    x_file_write_vec3(&recorder->file, &cam->collider.position);
    x_file_write_vec3(&recorder->file, &cam->collider.velocity);

    x_file_write_le_int32(&recorder->file, cam->angleX);
    x_file_write_le_int32(&recorder->file, cam->angleY);
}

static void save_brush_models(X_DemoRecorder* recorder)
{
    X_BspLevel* level = &recorder->cam->base.engineContext->currentLevel;
    for(int i = 0; i < level->totalModels; ++i)
        x_file_write_vec3(&recorder->file, &level->models[i].origin);
}

void x_demorecorder_init(X_DemoRecorder* recorder, X_CameraObject* cam, X_KeyState* keyState)
{
    recorder->cam = cam;
    recorder->keyState = keyState;
    recorder->recording = 0;
}

bool x_demorecorder_record(X_DemoRecorder* recorder, const char* outputFileName)
{
    recorder->totalFrames = 0;
    recorder->recording = 1;
    
    if(!x_file_open_writing(&recorder->file, outputFileName))
        return 0;
    
    x_file_write_le_int32(&recorder->file, X_DEMO_CURRENT_VERSION);
    x_file_write_le_int32(&recorder->file, recorder->totalFrames);
    
    return 1;
}

void x_demorecorder_cleanup(X_DemoRecorder* recorder)
{
    if(recorder->recording)
    {
        const int TOTAL_FRAMES_OFFSET = 4;
        x_file_seek(&recorder->file, TOTAL_FRAMES_OFFSET);
        x_file_write_le_int32(&recorder->file, recorder->totalFrames);
        x_file_close(&recorder->file);
    }
}

void x_demorecorder_save_frame(X_DemoRecorder* recorder)
{
    save_camera_state(recorder, recorder->cam);
    save_brush_models(recorder);
    ++recorder->totalFrames;
}

