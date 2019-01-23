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

#include "engine/Init.hpp"
#include "render/Screen.hpp"

class NspireScreenDriver
{
public:
    void init(X_Config& config);
    void cleanup();

    void setVideoMode(int screenW, int screenH, bool fullscreen);
    void update(X_Screen* screen);

private:
    void saveOldPalette();
    void restoreOldPalette();
    void setPalette(const X_Palette* palette);

    unsigned short* getPaletteRegisters()
    {
        return (unsigned short*)0xC0000200;
    }

    static unsigned short mapRgbToNspireColor(const unsigned char color[3])
    {
        unsigned short r = color[0] / 8;
        unsigned short g = color[1] / 8;
        unsigned short b = color[2] / 8;

        return b | (g << 5) | (r << 10);
    }

    int nativeResolutionW;
    int nativeResolutionH;
    SDL_Surface* surface;
    unsigned short oldColorPalette[256];
};

