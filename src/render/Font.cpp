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

#include "Font.hpp"
#include "error/Log.hpp"
#include "error/Error.hpp"
#include "memory/Alloc.h"

void X_Font::loadCharacter(const X_Texture& fontTex, int charRow, int charCol, X_Color* characterPixelsDest)
{
    int startX = charCol * charW;
    int startY = charRow * charH;
    
    for(int i = 0; i < charH; ++i)
    {
        for(int j = 0; j < charW; ++j)
        {
            *characterPixelsDest++ = fontTex.getTexel({ startX + j, startY + i });
        }
    }
}

void X_Font::loadCharacters(const X_Texture& fontTex)
{
    const int charsPerRow = 16;
    const int totalRows = 16;
    
    // Start at char 1 because 0 is the null terminator
    X_Color* nextCharacterPixels = pixels + charSize;
    
    for(int i = 0; i < totalRows; ++i)
    {
        for(int j = 0; j < charsPerRow; ++j)
        {
            int charId = j + i * charsPerRow;
            
            if(charId == charsPerRow * totalRows - 1)
                break;
            
            loadCharacter(fontTex, i, j, nextCharacterPixels);
            nextCharacterPixels += charSize;
        }
    }
    
}

bool X_Font::loadFromFile(const char* fileName)
{   
    X_Texture fontTex;
    if(!fontTex.loadFromFile(fileName))
    {
        Log::error("Failed to load font %s", fileName);
        return 0;
    }

    charW = fontTex.getW() / 16;
    charH = fontTex.getH() / 16;
    charSize = charW * charH;

    pixels = xalloc<X_Color>(fontTex.totalTexels());
    loadCharacters(fontTex);
    
    Log::info("Loaded font %s", fileName);
    
    return 1;
}

