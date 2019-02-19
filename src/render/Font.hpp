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

#include <string.h>

#include "Texture.hpp"

#define X_FONT_TOTAL_CHARS 256

class Font
{
public:
    Font() : charW(0), charH(0), pixels(nullptr) { }

    int getW() const { return charW; }
    int getH() const { return charH; }

    int calcWidthOfStr(const char* str) const
        { return strlen(str) * charW; }

    X_Color* getCharacterPixels(int c) const
        { return pixels + c * pixelsPerCharacter(); }

    bool loadFromFile(const char* fileName);

    ~Font()
    {
        xfree(pixels);
    }

private:
    int pixelsPerCharacter() const
        { return charSize; }

    void loadCharacter(const X_Texture& fontTex, int charRow, int charCol, X_Color* characterPixelsDest);
    void loadCharacters(const X_Texture& fontTex);

    int charW;
    int charH;
    int charSize;
    X_Color* pixels;
};

