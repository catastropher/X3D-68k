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

#include "render/Texture.hpp"

// Enables colord output for different types of logging
#define X_ENABLE_COLOR_LOG 1

// Cursor blink time for the console (in ms)
#define X_CONSOLE_CURSOR_BLINK_TIME 250

// Total time for the console to open or close (in ms)
#define X_CONSOLE_TOGGLE_TIME 250

// Extension to automatically add to all files that are opened for reading
#ifdef __nspire__
#define X_FILE_AUTO_ADDED_EXTENSION ".tns"
#else
#define X_FILE_AUTO_ADDED_EXTENSION ""
#endif

// Nspire has no implementation of clock() so we use SDL_GetTicks()
#ifdef __nspire__
#define X_GET_TIME_USING_SDL
#endif

#define X_SDL_SUPPORT

#ifndef __nspire__
#include "platform/pc/PcPlatform.hpp"
using Platform = PcPlatform;
#endif

#ifdef __nspire__
#include "platform/nspire/X_NspirePlatform.hpp"
using Platform = NspirePlatform;
#endif

