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

#include "X_NspireScreenDriver.hpp"
#include "error/X_log.h"

extern "C"
{
    void copy_screen(unsigned char* dest, unsigned char* src);
}

void NspireScreenDriver::init(X_Config& config)
{
    SDL_Init(SDL_INIT_VIDEO);

    lcd_init(SCR_320x240_8);
    memset(REAL_SCREEN_BASE_ADDRESS, 255, 320 * 240);  
    saveOldPalette();
    setPalette(x_palette_get_quake_palette());

    Log::info("Init nspire screen");
}

void NspireScreenDriver::cleanup()
{
    restoreOldPalette();
}

void NspireScreenDriver::saveOldPalette()
{
    unsigned short* palette = getPaletteRegisters();

    for(int i = 0; i < 256; ++i)
    {
        oldColorPalette[i] = palette[i];
    }
}

void NspireScreenDriver::restoreOldPalette()
{
    unsigned short* palette = getPaletteRegisters();

    for(int i = 0; i < 256; ++i)
    {
        palette[i] = oldColorPalette[i];
    }
}

void NspireScreenDriver::setPalette(const X_Palette* palette)
{
    unsigned int* paletteColor = (unsigned int*)getPaletteRegisters();

    for(int i = 0; i < 256; i += 2)
    {
        unsigned int lo = mapRgbToNspireColor(&palette->colorRGB[i][0]);
        unsigned int hi = mapRgbToNspireColor(&palette->colorRGB[i + 1][0]);
        
        *paletteColor++ = (lo | (hi << 16));
    }
}

void NspireScreenDriver::update(X_Screen* screen)
{
    copy_screen((unsigned char*)REAL_SCREEN_BASE_ADDRESS, screen->canvas.getTexels());
}

