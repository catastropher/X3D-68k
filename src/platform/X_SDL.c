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

#include "X_Platform.h"
#include "X_SDL.h"
#include "system/X_Keys.h"

#define TOTAL_SDL_KEYS 322
#define INVALID_KEY -1

static int x3dKeyMap[TOTAL_SDL_KEYS];

static void build_key_map(void)
{
    int* keyMap = x3dKeyMap;
    
    for(int i = 0; i < TOTAL_SDL_KEYS; ++i)
        keyMap[i] = INVALID_KEY;
    
    for(int i = 'a'; i <= 'z'; ++i)
        keyMap[i] = i;
    
    for(int i = 'A'; i <= 'Z'; ++i)
        keyMap[i] = i;
    
    for(int i = '0'; i <= '9'; ++i)
        keyMap[i] = i;
    
    keyMap[SDLK_SPACE] = ' ';
    
    keyMap[SDLK_LSHIFT] = keyMap[SDLK_RSHIFT] = X_KEY_SHIFT;
    keyMap[SDLK_RETURN] = '\n';
    keyMap[SDLK_ESCAPE] = X_KEY_ESCAPE;
    keyMap[SDLK_BACKSPACE] = '\b';
    keyMap[SDLK_TAB] = '\t';
    
    const int AZERTY_SUPERSCRIPT_2 = 178;
    keyMap[SDLK_BACKQUOTE] = keyMap['~'] = keyMap[AZERTY_SUPERSCRIPT_2] = keyMap['9'] = X_KEY_OPEN_CONSOLE;
    
    keyMap[SDLK_UP] = X_KEY_UP;
    keyMap[SDLK_DOWN] = X_KEY_DOWN;
    keyMap[SDLK_LEFT] = X_KEY_LEFT;
    keyMap[SDLK_RIGHT] = X_KEY_RIGHT;

    const char symbols[] = "!@#$%^&*()[]{}\\|:;'\",.<>/?-_=+";
    for(int i = 0; i < strlen(symbols); ++i)
        keyMap[(int)symbols[i]] = symbols[i];
}

void x_sdl_init_keys(X_EngineContext* engineContext, _Bool enableUnicode)
{
    build_key_map();
    
    if(enableUnicode)
        SDL_EnableUNICODE(SDL_ENABLE);
}

void x_sdl_cleanup_keys(X_EngineContext* engineContext)
{
    
}

static int translate_sdl_key_to_x3d_key(SDLKey key)
{
    if(key < 0 || key > TOTAL_SDL_KEYS)
        return INVALID_KEY;
    
    return x3dKeyMap[key];
}

void x_sdl_handle_keys(X_EngineContext* engineContext)
{
    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        if(ev.type == SDL_KEYDOWN)
        {
            SDLKey key, unicodeCharacter;
            x_platform_sdl_extract_key_from_event(&ev, &key, &unicodeCharacter);
            
            int x3dKey = translate_sdl_key_to_x3d_key(key);
            int x3dUnicodeCharacter = translate_sdl_key_to_x3d_key(key);
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_press(&engineContext->keystate, x3dKey, x3dUnicodeCharacter);
        }
        else if(ev.type == SDL_KEYUP)
        {
            int sdlKey = ev.key.keysym.sym;
            int x3dKey = translate_sdl_key_to_x3d_key(sdlKey);
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_release(&engineContext->keystate, x3dKey);
        }
    }
}

