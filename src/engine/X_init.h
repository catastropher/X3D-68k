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

#include "render/X_Screen.h"

struct X_Config
{
    X_Config()
    {
        fov = 0;
        screenW = 0;
        screenH = 0;
        fullscreen = 0;
        
        screenHandlers.displayFrame = NULL;
        screenHandlers.isValidResolution = NULL;
        screenHandlers.restartVideo = NULL;
        screenHandlers.userData = NULL;
        
        path = NULL;
    }

    X_Config& screenSize(int w, int h)
    {
        screenW = w;
        screenH = h;

        return *this;
    }

    X_Config& fieldOfView(fp fov)
    {
        this->fov = fov.toFp16x16();

        return *this;
    }

    X_Config& programPath(const char* path)
    {
        this->path = path;

        return *this;
    }

    X_Config& screenFullscreen()
    {
        fullscreen = true;

        return *this;
    }

    X_Config& colorPalette(const X_Palette* palette)
    {
        palette = palette;

        return *this;
    }

    X_Config& useQuakeColorPalette()
    {
        palette = x_palette_get_quake_palette();

        return *this;
    }

    X_ScreenEventHandlers screenHandlers;
    const X_Palette* palette;
    
    int screenW;
    int screenH;
    x_fp16x16 fov;
    bool fullscreen;
    
    const char* path;
};

void x_config_init(X_Config* config);

void x_config_set_screen_defaults(X_Config* config, int screenW, int screenH, x_fp16x16 fov, bool fullscreen);
void x_config_set_screen_display_frame_callback(X_Config* config, void (*displayFrameCallback)(struct X_Screen* screen, void* userData));
void x_config_set_screen_restart_video_callback(X_Config* config, void (*restartVideoCallback)(struct X_EngineContext* context, void* userData));
void x_config_set_screen_is_valid_resolution_callback(X_Config* config, bool (*isValidResolutionCallback)(int w, int h));
void x_config_set_screen_user_data(X_Config* config, void* userData);
void x_config_set_screen_cleanup_video_callback(X_Config* config, void (*cleanupVideoCallback)(struct X_EngineContext* context, void* userData));

static inline void x_config_set_program_path(X_Config* config, const char* programPath)
{
    programPath = programPath;
}

static inline void x_config_screen_set_palette(X_Config* config, const X_Palette* palette)
{
    palette = palette;
}

