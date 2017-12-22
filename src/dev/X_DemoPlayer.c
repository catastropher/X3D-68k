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

#include "X_DemoPlayer.h"
#include "error/X_log.h"
#include "engine/X_EngineContext.h"

void x_demoplayer_init(X_DemoPlayer* player, X_CameraObject* cam, X_KeyState* keyState)
{
    player->cam = cam;
    player->keyState = keyState;
    
    player->totalFrames = 0;
    player->currentFrame = 0;
    
    player->file.file = NULL;       // FIXME: need way to correcly initialize file
    player->file.flags = 0;
}

static void read_camera_state(X_CameraObject* cam, X_File* file)
{
    x_file_read_mat4x4(file, &cam->viewMatrix);
    
    x_file_read_vec3(file, &cam->collider.position);
    x_file_read_vec3(file, &cam->collider.velocity);
    
    cam->angleX = x_file_read_le_int32(file);
    cam->angleY = x_file_read_le_int32(file);
    
    x_cameraobject_update_view(cam);
}

static void read_brush_models(X_DemoPlayer* player)
{
    X_BspLevel* level = &player->cam->base.engineContext->currentLevel;
    for(int i = 0; i < level->totalModels; ++i)
        x_file_read_vec3(&player->file, &level->models[i].origin);
}

_Bool x_demoplayer_play(X_DemoPlayer* player, const char* fileName)
{
    player->totalFrames = 0;
    player->currentFrame = 0;
    
    if(x_file_is_open(&player->file))
        x_file_close(&player->file);
    
    if(!x_file_open_reading(&player->file, fileName))
        return 0;
    
    int version = x_file_read_le_int32(&player->file);
    if(version != X_DEMO_CURRENT_VERSION)
    {
        x_log_error("Demo has bad version %d (expected %d)", version, X_DEMO_CURRENT_VERSION);
        return 0;
    }
    
    player->totalFrames = x_file_read_le_int32(&player->file);
    x_log("Total frames: %d\n", player->totalFrames);
    
    return 1;
}

void x_demoplayer_play_frame(X_DemoPlayer* player)
{
    if(player->currentFrame >= player->totalFrames)
    {
        x_file_close(&player->file);
        return;
    }
    
    read_camera_state(player->cam, &player->file);
    read_brush_models(player);
    
    ++player->currentFrame;
}

void x_demoplayer_cleanup(X_DemoPlayer* player)
{
    if(player->file.file)
        x_file_close(&player->file);
}

