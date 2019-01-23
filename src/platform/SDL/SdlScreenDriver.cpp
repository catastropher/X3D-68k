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

#include "SdlScreenDriver.hpp"
#include "error/Error.hpp"
#include "util/Sdl.hpp"

void SdlScreenDriver::init(X_Config& config)
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        x_system_error("Failed to init SDL video");
    }

    const SDL_VideoInfo* info = SDL_GetVideoInfo();
        
    nativeResolutionW = info->current_w;
    nativeResolutionH = info->current_h;

    setVideoMode(config.screen->w, config.screen->h, config.screen->fullscreen);
}

void SdlScreenDriver::setVideoMode(int screenW, int screenH, bool fullscreen)
{
    int flags = SDL_SWSURFACE;

    if(fullscreen)
    {
        flags |= SDL_FULLSCREEN;
    }

    surface = SDL_SetVideoMode(screenW, screenH, 32, flags);
    if(!surface)
    {
        x_system_error("Failed to set SDL video mode");
    }
}

void SdlScreenDriver::cleanup()
{
    SDL_SetVideoMode(nativeResolutionW, nativeResolutionH, 32, 0);
}

void SdlScreenDriver::update(X_Screen* screen)
{
    x_texture_to_sdl_surface(&screen->canvas, screen->palette, surface);
    SDL_Flip(surface);
}

