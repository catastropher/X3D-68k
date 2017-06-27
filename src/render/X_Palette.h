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

typedef struct X_Palette
{
    unsigned char colorRGB[256][3];
} X_Palette;

static inline void x_palette_get_rgb(const X_Palette* palette, X_Color color, unsigned char* r, unsigned char* g, unsigned char* b)
{
    int colorAsInt = color;
    *r = palette->colorRGB[colorAsInt][0];
    *g = palette->colorRGB[colorAsInt][1];
    *b = palette->colorRGB[colorAsInt][2];
}

