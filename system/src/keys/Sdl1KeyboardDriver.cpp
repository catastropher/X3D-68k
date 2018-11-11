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

#include "Sdl1KeyboardDriver.hpp"

namespace X3D
{
    void Sdl1KeyboardDriver::init()
    {
        keyState.clear();
    }

    void Sdl1KeyboardDriver::cleanup()
    {

    }

    void Sdl1KeyboardDriver::sendSdlKeyPress(int sdlKey)
    {
        int x3dKey = mapSdlKeyToX3dKey(sdlKey);

        if(x3dKey != KEY_INVALID)
        {
            sendKeyPress(x3dKey);
        }
    }

    void Sdl1KeyboardDriver::sendSdlKeyRelease(int sdlKey)
    {
        int x3dKey = mapSdlKeyToX3dKey(sdlKey);

        if(x3dKey != KEY_INVALID)
        {
            sendKeyRelease(x3dKey);
        }
    }

    int Sdl1KeyboardDriver::mapSdlKeyToX3dKey(int sdlKey)
    {
        if(isalnum(sdlKey) || sdlKey == ' ')
        {
            return sdlKey;
        }

        switch(sdlKey)
        {
            case SDLK_RETURN:   return KEY_ENTER;
            case SDLK_TAB:      return KEY_TAB;
            case SDLK_UP:       return KEY_UP;
            case SDLK_DOWN:     return KEY_DOWN;
            case SDLK_LEFT:     return KEY_LEFT;
            case SDLK_RIGHT:    return KEY_RIGHT;
            case SDLK_ESCAPE:   return KEY_ESCAPE;
        }

        return KEY_INVALID;
    }

    void Sdl1KeyboardDriver::update()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    sendSdlKeyPress(event.key.keysym.sym);
                    break;

                case SDL_KEYUP:
                    sendSdlKeyRelease(event.key.keysym.sym);
                    break;
            }
        }
    }
}

