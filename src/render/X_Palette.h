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

#include "X_Texture.h"

struct X_Screen;

typedef struct X_Palette
{
    unsigned char colorRGB[256][3];
    
    X_Color brightRed;
    X_Color darkRed;
    
    X_Color lightBlue;
    X_Color darkBlue;
    
    X_Color lightGreen;
    X_Color darkGreen;
    
    X_Color lightGray;
    X_Color darkGray;
    
    X_Color black;
    X_Color white;
    
    unsigned int grayscaleTable[256];
} X_Palette;

X_Color x_palette_get_closest_color_from_rgb(const X_Palette* palette, unsigned char r, unsigned char g, unsigned char b);
const X_Palette* x_palette_get_quake_palette(void);

void x_palette_visualize(struct X_Screen* screen);

static inline void x_palette_get_rgb(const X_Palette* palette, X_Color color, unsigned char* r, unsigned char* g, unsigned char* b)
{
    int colorAsInt = color;
    *r = palette->colorRGB[colorAsInt][0];
    *g = palette->colorRGB[colorAsInt][1];
    *b = palette->colorRGB[colorAsInt][2];
}

