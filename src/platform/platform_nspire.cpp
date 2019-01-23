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

#include <libndls.h>

#include "Platform.hpp"
#include "SDL.h"
#include "socket_nspire.h"

static void init_net()
{
    //x_socket_nspire_register_interface();
}

void x_platform_init(X_EngineContext* engineContext, X_Config* config)
{
    x_sdl_init_keys(engineContext, 0);
    init_net();

    Log::info("Initialized nspire\n");
}

void x_platform_cleanup(X_EngineContext* engineContext)
{
    x_sdl_cleanup_keys(engineContext);
}

void x_platform_handle_keys(X_EngineContext* engineContext)
{
    x_sdl_handle_keys(engineContext);
}

void x_platform_handle_mouse(X_EngineContext* engineContext)
{
    X_MouseState* state = engineContext->getMouseState();
    touchpad_report_t report;
    touchpad_info_t* info = touchpad_getinfo();
    
    _Bool success = info && touchpad_scan(&report) == 0;
    if(!success)
        return;
    
    X_Vec2 center = x_screen_center(engineContext->getScreen());
    
    if(!report.contact)
    {
        x_mousestate_update_pos(state, center);
    }
    
    X_Vec2 pos = x_vec2_make(report.x - info->width / 2, report.y - info->height / 2);
    
    int scale = 20;
    
    pos.x = pos.x * scale / info->width;
    pos.y = pos.y * scale / info->height;
    
    pos = x_vec2_add(&center, &pos);
        
    x_mousestate_update_pos(state, pos);
}

void x_platform_sdl_extract_key_from_event(SDL_Event* ev, SDLKey* key, SDLKey* unicodeCharacter)
{
    *key = ev->key.keysym.sym;
    *unicodeCharacter = ev->key.keysym.sym;
}

void x_platform_mouse_set_position(X_Vec2 pos)
{
    
}

void x_platform_mouse_show_cursor(_Bool showCursor)
{
    
}

