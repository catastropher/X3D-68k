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

#include "Platform.hpp"
#include "SDL.h"

void x_platform_init(EngineContext* engineContext, X_Config* config)
{
    x_sdl_init_keys(engineContext, 1);
}

void x_platform_cleanup(EngineContext* engineContext)
{
    x_sdl_cleanup_keys(engineContext);
    x_sdl_mouse_show_cursor(1);
}

void x_platform_handle_keys(EngineContext* engineContext)
{
    x_sdl_handle_keys(engineContext);
}

void x_platform_handle_mouse(EngineContext* engineContext)
{
    x_sdl_handle_mouse(engineContext);
}

void x_platform_sdl_extract_key_from_event(SDL_Event* ev, SDLKey* key, SDLKey* unicodeCharacter)
{
    *key = ev->key.keysym.sym;
    *unicodeCharacter = (SDLKey)ev->key.keysym.unicode;
    
    // FIXME: unicode is being initialized before SDL!
    SDL_EnableUNICODE(SDL_ENABLE);
    
    if(!isprint(*key))
        *unicodeCharacter = *key;    
}

void x_platform_mouse_set_position(Vec2 pos)
{
    x_sdl_mouse_set_position(pos);
}

void x_platform_mouse_show_cursor(bool showCursor)
{
    x_sdl_mouse_show_cursor(showCursor);
}

