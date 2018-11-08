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

#include "RasterFont.hpp"
#include "Texture.hpp"

namespace X3D
{
    void RasterFont::loadFromFile(FilePath& path)
    {
        const int CHARS_IN_ROW = 16;
        const int TOTAL_COLUMNS = 16;
        const int TOTAL_CHARS = 256;

        ManagedTexture texture(AllocationSource::zone);
        texture.loadFromFile(path);

        charW = texture.getW() / CHARS_IN_ROW;
        charH = texture.getH() / TOTAL_COLUMNS;
        texelsPerChar = charW * charH;

        ZoneAllocator* zone = ServiceLocator::get<ZoneAllocator>();

        fontTexels = zone->alloc<Color>(texelsPerChar * TOTAL_CHARS);

        for(int c = 1; c < TOTAL_CHARS; ++c)
        {
            int charId = c - 1;

            int charRow = charId / CHARS_IN_ROW;
            int charCol = charId % CHARS_IN_ROW;

            Color* charTexels = fontTexels + c * texelsPerChar;

            int texelsRow = charRow * charH;
            int texelsCol = charCol * charW;

            for(int row = 0; row < charH; ++row)
            {
                for(int col = 0; col < charW; ++col)
                {
                    charTexels[row * charW + col] = texture.getTexel(texelsCol + col, texelsRow + row);
                }
            }
        }
    }
}

