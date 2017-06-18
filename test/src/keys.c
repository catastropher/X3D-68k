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

#define TOTAL_SDL_KEYS 322

_Bool keyState[TOTAL_SDL_KEYS];

void handle_key_events()
{
    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        if(ev.type == SDL_KEYDOWN)
        {
            keyState[ev.key.keysym.sym] = 1;
        }
        else if(ev.type == SDL_KEYUP)
        {
            keyState[ev.key.keysym.sym] = 0;
        }
    }
}

_Bool key_is_down(int sdlKey)
{
    return keyState[sdlKey];
}

