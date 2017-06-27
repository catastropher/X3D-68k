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

#define X_FONT_TOTAL_CHARS 256

typedef struct X_Font
{
    int charW;
    int charH;
    size_t charSize;
    X_Color* pixels;
} X_Font;

_Bool x_font_load_from_xtex_file(X_Font* font, const char* fileName, int fontWidth, int fontHeight);
void x_font_cleanup(X_Font* font);

static inline const X_Color* x_font_get_character_pixels(const X_Font* font, int charId)
{
    return font->pixels + charId * font->charSize;
}


