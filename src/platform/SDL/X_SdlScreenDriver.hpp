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

#include "engine/X_init.h"
#include "render/X_Screen.h"

class SdlScreenDriver
{
public:
    void init(X_Config& config);
    void cleanup();

    void setVideoMode(int screenW, int screenH, bool fullscreen);
    void update(X_Screen* screen);

private:
    int nativeResolutionW;
    int nativeResolutionH;
    SDL_Surface* surface;
};

