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

#include <SDL/SDL.h>
#include <X3D/X3D.h>

#define TOTAL_SDL_KEYS 322

#define INVALID_KEY -1

_Bool keyState[TOTAL_SDL_KEYS];

void init_keys()
{
    SDL_EnableUNICODE(SDL_ENABLE);
}

static _Bool is_symbol_key(int c)
{
    const char symbols[] = "!@#$%^&*()[]{}\\|:;'\",.<>/?-_=+";
    
    for(int i = 0; i < sizeof(symbols); ++i)
    {
        if(c == symbols[i])
            return 1;
    }
    
    return 0;
}

int convert_sdl_key_to_x3d_key(int sdlKey)
{
    if(sdlKey == SDLK_RETURN)
        return '\n';
    
    if(sdlKey == SDLK_BACKSPACE)
        return '\b';
    
    if(sdlKey == SDLK_TAB)
        return '\t';
    
    if(sdlKey == SDLK_BACKQUOTE || sdlKey == '~' || sdlKey == 178)
        return X_KEY_OPEN_CONSOLE;
    
    if(sdlKey == SDLK_SPACE)
        return ' ';
    
    if(sdlKey == SDLK_LSHIFT || sdlKey == SDLK_RSHIFT)
        return X_KEY_SHIFT;
    
    if(sdlKey == SDLK_UP)
        return X_KEY_UP;
    
    if(sdlKey == SDLK_DOWN)
        return X_KEY_DOWN;
    
    if(sdlKey == SDLK_LEFT)
        return X_KEY_LEFT;
    
    if(sdlKey == SDLK_RIGHT)
        return X_KEY_RIGHT;
    
    if(is_symbol_key(sdlKey))
        return sdlKey;
    
    if(isalnum(sdlKey))
        return sdlKey;
    
    if(sdlKey == '.' || sdlKey == '/')
        return sdlKey;
    
    return INVALID_KEY;
}

void handle_key_events(X_EngineContext* context)
{
    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        if(ev.type == SDL_KEYDOWN)
        {
            keyState[ev.key.keysym.sym] = 1;
            
            int sdlKey = (context->keystate.textInputMode ? ev.key.keysym.unicode : ev.key.keysym.sym);
            int x3dKey = convert_sdl_key_to_x3d_key(sdlKey);
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_press(&context->keystate, x3dKey);
        }
        else if(ev.type == SDL_KEYUP)
        {
            keyState[ev.key.keysym.sym] = 0;
            
            int sdlKey = (context->keystate.textInputMode ? ev.key.keysym.unicode : ev.key.keysym.sym);
            int x3dKey = convert_sdl_key_to_x3d_key(sdlKey);
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_release(&context->keystate, x3dKey);
        }
    }
}

_Bool key_is_down(int sdlKey)
{
    return keyState[sdlKey];
}

