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

#include "X_init.h"

void x_config_init(X_Config* config)
{
    config->fov = 0;
    config->screenW = 0;
    config->screenH = 0;
    config->fullscreen = 0;
    
    config->screenHandlers.displayFrame = NULL;
    config->screenHandlers.isValidResolution = NULL;
    config->screenHandlers.restartVideo = NULL;
    config->screenHandlers.userData = NULL;
    
    config->programPath = NULL;
}

void x_config_set_screen_defaults(X_Config* config, int screenW, int screenH, int fov, _Bool fullscreen)
{
    config->screenW = screenW;
    config->screenH = screenH;
    config->fov = fov;
    config->fullscreen = fullscreen;
}

void x_config_set_screen_display_frame_callback(X_Config* config, void (*displayFrameCallback)(struct X_Screen* screen, void* userData))
{
    config->screenHandlers.displayFrame = displayFrameCallback;
}

void x_config_set_screen_restart_video_callback(X_Config* config, void (*restartVideoCallback)(struct X_EngineContext* context, void* userData))
{
    config->screenHandlers.restartVideo = restartVideoCallback;
}

void x_config_set_screen_is_valid_resolution_callback(X_Config* config, _Bool (*isValidResolutionCallback)(int w, int h))
{
    config->screenHandlers.isValidResolution = isValidResolutionCallback;
}

void x_config_set_screen_user_data(X_Config* config, void* userData)
{
    config->screenHandlers.userData = userData;
}

