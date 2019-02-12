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

#include <SDL/SDL.h>

#include "engine/EngineContext.hpp"

#define X_SDL_REQUIRED

void x_sdl_init_keys(EngineContext* engineContext, bool enableUnicode);
void x_sdl_cleanup_keys(EngineContext* engineContext);
void x_sdl_handle_keys(EngineContext* engineContext);

void x_sdl_handle_mouse(EngineContext* engineContext);
void x_sdl_mouse_set_position(Vec2 pos);
void x_sdl_mouse_show_cursor(bool showCursor);

X_SDL_REQUIRED void x_platform_sdl_extract_key_from_event(SDL_Event* ev, SDLKey* key, SDLKey* unicodeCharacter);

