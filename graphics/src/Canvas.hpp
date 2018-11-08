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

#include <X3D-math.hpp>

#include "Texture.hpp"
#include "RasterFont.hpp"
#include "ColorMap.hpp"
#include "LightingTable.hpp"

namespace X3D
{
    struct BlitTexture
    {
        // TODO: fill in
    };

    class Canvas
    {
    public:
        Canvas(Texture* texture_)
        {
            setTexture(texture_);
        }

        void setTexture(Texture* texture_)
        {
            texture = texture_;
            maxX = fp::fromInt(texture->getW() - 1);
            maxY = fp::fromInt(texture->getH() - 1);
        }

        void setLightingTable(LightingTable* lightingTable_)
        {
            lightingTable = lightingTable_;
        }

        void clamp(Vec2& v)
        {
            v.x = X3D::clamp(v.x, fp(0), maxX);
            v.y = X3D::clamp(v.y, fp(0), maxY);
        }

        void clamp(Vec2i& v)
        {
            v.x = X3D::clamp(v.x, 0, texture->getW() - 1);
            v.y = X3D::clamp(v.y, 0, texture->getH() - 1);
        }

        void fill(Color color);

        void drawLine(Vec2i start, Vec2i end, Color color);
        void drawLineAntiAlias(Vec2 start, Vec2 end, Color color);  // TODO
        void drawHLine(int x1, int x2, int y, Color color);
        void drawVLine(int y1, int y2, int x, Color color);

        void drawRectFilled(const BoundRecti& rect, Color color); // TODO
        void drawRectOutlined(const BoundRecti& rect, Color color);

        void drawCircleFilled(Vec2i center, int radius, Color color);   // TODO
        void drawCircleOutlined(Vec2i center, int radius, Color color); // TODO

        void drawPoint(Vec2i pos, Color color);     // TODO
        Color getPoint(Vec2i pos);  // TODO

        void blitTexture(const Texture& tex, Vec2i pos);
        void blitTextureClipped(const Texture& tex, Vec2i pos);

        void drawChar(RasterFont& font, char c);    // TODO
        void drawString(const char* str, const RasterFont& font, Vec2i pos); // TODO


        void applyColorMap(ColorMap& map, Vec2i topLeft, Vec2i bottomRight);    // TODO

        void drawPalette(Palette* palette, int x, int y, int colorSize);

        void saveToFile(FilePath& file);    // TODO

    private:
        Texture* texture;
        fp maxX;
        fp maxY;
        LightingTable* lightingTable;
    };
}

