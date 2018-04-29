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

struct ScreenConfig
{
    ScreenConfig() : fov(0)     // Because fp doesn't have a default constructor yet
    {
        screenHandlers.displayFrame = NULL;
        screenHandlers.isValidResolution = NULL;
        screenHandlers.restartVideo = NULL;
        screenHandlers.userData = NULL;

        fov = 0;
        w = 320;
        h = 240;
        fullscreen = false;
    }

    ScreenConfig& displayFrameCallback(void (*callback)(struct X_Screen* screen, void* userData))
    {
        screenHandlers.displayFrame = callback;

        return *this;
    }

    ScreenConfig& restartVideoCallback(void (*callback)(struct X_EngineContext* context, void* userData))
    {
        screenHandlers.restartVideo = callback;

        return *this;
    }

    ScreenConfig& isValidResolutionCallback(bool (*callback)(int w, int h))
    {
        screenHandlers.isValidResolution = callback;

        return *this;
    }

    ScreenConfig& cleanupVideoCallback(void (*callback)(struct X_EngineContext* context, void* userData))
    {
        screenHandlers.cleanupVideo = callback;

        return *this;
    }

    ScreenConfig& userData(void* data)
    {
        screenHandlers.userData = data;

        return *this;
    }

    ScreenConfig& resolution(int w, int h)
    {
        this->w = w;
        this->h = h;

        return *this;
    }

    ScreenConfig& fieldOfView(fp fov)
    {
        this->fov = fov;

        return *this;
    }

    ScreenConfig& enableFullscreen()
    {
        fullscreen = true;

        return *this;
    }

    ScreenConfig& colorPalette(const X_Palette* palette)
    {
        palette = palette;

        return *this;
    }

    ScreenConfig& useQuakeColorPalette()
    {
        palette = x_palette_get_quake_palette();

        return *this;
    }

    int w;
    int h;
    fp fov;
    bool fullscreen;
    const X_Palette* palette;

    X_ScreenEventHandlers screenHandlers;
};

struct X_Config
{
    X_Config()
    {
        path = nullptr;
        font = "font.xtex";
    }

    X_Config& screenConfig(ScreenConfig& config)
    {
        screen = &config;

        return *this;
    }

    X_Config& programPath(const char* path)
    {
        this->path = path;

        return *this;
    }

    X_Config& defaultFont(const char* fileName)
    {
        font = fileName;

        return *this;
    }
    
    const char* path;
    const char* font;
    ScreenConfig* screen;
};

void x_config_init(X_Config* config);

