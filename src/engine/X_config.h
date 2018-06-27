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

#include "render/X_Texture.h"

// Enables colord output for different types of logging
#define X_ENABLE_COLOR_LOG 1

// Cursor blink time for the console (in ms)
#define X_CONSOLE_CURSOR_BLINK_TIME 250

// Total time for the console to open or close (in ms)
#define X_CONSOLE_TOGGLE_TIME 250

// Extension to automatically add to all files that are opened for reading
#ifdef __nspire__
#define X_FILE_AUTO_ADDED_EXTENSION ".tns"
#endif

// Nspire has no implementation of clock() so we use SDL_GetTicks()
#ifdef __nspire__
#define X_GET_TIME_USING_SDL
#endif

#define X_SDL_SUPPORT

#ifndef __nspire__
#include "platform/pc/X_PcPlatform.hpp"
using Platform = PcPlatform;
#endif

#ifdef __nspire__
#include "platform/nspire/X_NspirePlatform.hpp"
using Platform = NspirePlatform;
#endif

struct ConsoleConfig
{
    X_Color backgroundColor;
    X_Color lineColor;
};

struct ScreenConfig2
{
    const X_Palette* palette;
    int screenW;
    int screenH;
    bool fullscreen;
};

struct MemoryConfig
{
    int hunkSize;
    int zoneSize;
};

struct Config
{
    ConsoleConfig console;
    MemoryConfig memory;

    // JsonValue* configJson;

    // JsonValue& operator[](const char* name)
    // {
    //     return (*configJson)[name];
    // }

    const char* configFile;
};

struct SystemConfig
{
    const char* programPath = nullptr;
    const char* logFile = "engine.log";
    int hunkSize = 8 * 1024 * 1024;
    int zoneSize = 1024 * 1024;
    bool enableLogging = true;
};

