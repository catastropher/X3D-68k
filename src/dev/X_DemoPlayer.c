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

void x_demoplayer_init(X_DemoPlayer* player, X_CameraObject* cam, X_KeyState* keyState)
{
    player->cam = cam;
    player->keyState = keyState;
    
    player->frames = NULL;
    player->totalFrames = 0;
    player->currentFrame = 0;
}

static void read_camera_start(X_CameraObject* cam, X_File* file)
{
    x_file_read_mat4x4(file, &cam->viewMatrix);
    
    x_file_read_vec3(file, &cam->base.position);
    x_file_read_vec3(file, &cam->base.velocity);
    
    cam->angleX = x_file_read_le_int32(file);
    cam->angleY = x_file_read_le_int32(file);
}

static void read_frames(X_DemoPlayer* player, X_File* file)
{
    player->frames = x_malloc(player->totalFrames * sizeof(X_DemoPlayerFrame));
    
    for(int i = 0; i < player->totalFrames; ++i)
        x_file_read_buf(file, X_KEY_BITARRAY_SIZE, &player->frames[i].keyState);
}

_Bool x_demoplayer_play(X_DemoPlayer* player, const char* fileName)
{
    if(player->frames)
        x_free(player->frames);
    
    player->frames = NULL;
    player->totalFrames = 0;
    player->currentFrame = 0;
    
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return 0;
    
    int version = x_file_read_le_int32(&file);
    if(version != X_DEMO_CURRENT_VERSION)
    {
        x_log_error("Demo has bad version %d (expected %d)", version, X_DEMO_CURRENT_VERSION);
        return 0;
    }
    
    player->totalFrames = x_file_read_le_int32(&file);
    printf("Total frames: %d\n", player->totalFrames);
    read_camera_start(player->cam, &file);
    read_frames(player, &file);
    
    x_file_close(&file);
    
    return 1;
}

void x_demoplayer_play_frame(X_DemoPlayer* player)
{
    if(!x_demoplayer_is_playing(player))
        return;
    
    X_DemoPlayerFrame* frame = player->frames + player->currentFrame++;
    for(int i = 0; i < X_TOTAL_KEYS; ++i)
        player->keyState->keyDown[i] = frame->keyState[i / 8] & (1 << (i % 8));
}

void x_demoplayer_cleanup(X_DemoPlayer* player)
{
    x_free(player->frames);
}

