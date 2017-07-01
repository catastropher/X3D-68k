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

#include "X_Font.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "memory/X_alloc.h"

static size_t calculate_character_size(const X_Font* font)
{
    return font->charW * font->charH * sizeof(X_Color);
}

static size_t calculate_font_pixels_size(const X_Font* font)
{
    return X_FONT_TOTAL_CHARS * font->charSize;
}

static void load_character(X_Font* font, const X_Texture* fontTex, int charRow, int charCol, X_Color* characterPixelsDest)
{
    int startX = charCol * font->charW;
    int startY = charRow * font->charH;
    
    for(int i = 0; i < font->charH; ++i)
        for(int j = 0; j < font->charW; ++j)
            *characterPixelsDest++ = x_texture_get_texel(fontTex, startX + j, startY + i);
}

static void load_characters(X_Font* font, const X_Texture* fontTex)
{
    int charsPerRow = x_texture_w(fontTex) / font->charW;
    
    x_assert(x_texture_w(fontTex) % font->charW == 0, "Font texture width not multiple of font width");
    x_assert(x_texture_h(fontTex) % font->charH == 0, "Font texture height not multiple of font height");
    
    int totalRows = x_texture_h(fontTex) / font->charH;
    x_assert(totalRows * charsPerRow == X_FONT_TOTAL_CHARS, "Wrong number of chars in font");
    
    // Start at char 1 because 0 is the null terminator
    X_Color* nextCharacterPixels = font->pixels + font->charSize;
    
    for(int i = 0; i < totalRows; ++i)
    {
        for(int j = 0; j < charsPerRow; ++j)
        {
            int charId = j + i * charsPerRow;
            
            if(charId == charsPerRow * totalRows - 1)
                break;
            
            load_character(font, fontTex, i, j, nextCharacterPixels);
            nextCharacterPixels += font->charSize;
        }
    }
    
}

_Bool x_font_load_from_xtex_file(X_Font* font, const char* fileName, int fontWidth, int fontHeight)
{
    font->charW = fontWidth;
    font->charH = fontHeight;
    
    X_Texture fontTex;
    if(!x_texture_load_from_xtex_file(&fontTex, fileName))
    {
        x_log_error("Failed to load font %s", fileName);
        return 0;
    }
    
    font->charSize = calculate_character_size(font);
    font->pixels = x_malloc(calculate_font_pixels_size(font));
    load_characters(font, &fontTex);
    x_texture_cleanup(&fontTex);
    
    x_log("Loaded font %s", fileName);
    
    return 1;
}

void x_font_cleanup(X_Font* font)
{
    x_free(font->pixels);
    font->pixels = NULL;
}

