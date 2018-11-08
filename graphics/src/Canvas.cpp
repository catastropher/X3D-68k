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

#include "Canvas.hpp"

namespace X3D
{
    void Canvas::fill(Color color)
    {
        memset(texture->getTexels(), color, texture->totalTexels());
    }

    void Canvas::drawLine(Vec2i start, Vec2i end, Color color)
    {
        clamp(start);
        clamp(end);

        int dx = end.x - start.x;
        int dy = end.y - start.y;

        int sx = start.x < end.x ? 1 : -1;
        int sy = start.y < end.y ? 1 : -1;

        int err = (dx > dy ? dx : -dy) / 2;
        
        Vec2i pos = start;

        do
        {
            texture->setTexel(pos, color);

            if(pos == end)
            {
                break;
            }

            int oldError = err;
            if(oldError > -dx)
            {
                err -= dy;
                pos.x += sx;
            }

            if(oldError < dy)
            {
                err += dx;
                pos.y += sy;
            }
        } while(true);
    }

    void Canvas::drawPalette(Palette* palette, int x, int y, int colorSize)
    {
        for(int i = 0; i < 16; ++i)
        {
            for(int j = 0; j < 16; ++j)
            {
                int y = i * colorSize;
                int x = j * colorSize;
                Color color = i * 16 + j;
                
                for(int k = 0; k < 8; ++k)
                {
                    for(int d = 0; d < 8; ++d)
                    {
                        texture->setTexel(x + d, y + k, color);
                    }
                }
            }
        }
    }

    void Canvas::drawVLine(int y1, int y2, int x, Color color)
    {
        if(x >= texture->getW() || x < 0)
        {
            return;
        }
        
        if(y1 > y2)
        {
            std::swap(y1, y2);
        }

        if(y1 < 0)
        {
            y1 = 0;
        }

        if(y2 >= texture->getH())
        {
            y2 = texture->getH() - 1;
        }

        for (int i = y1; i <= y2; ++i)
        {
            texture->setTexel(x, i, color);
        }

    }

    void Canvas::drawHLine(int x1, int x2, int y, Color color)
    {
        if(y >= texture->getH() || y < 0)
        {
            return;
        }

        if(x1 > x2)
        {
            std::swap(x1, x2);
        }

        if(x1 < 0)
        {
            x1 = 0;
        }

        if(x2 >= texture->getW())
        {
            x2 = texture->getW() - 1;
        }

        int length = x2 - x1 + 1;

        if(length < 8)
        {
            for(int i = x1; i <= x2; ++i)
            {
                texture->setTexel(i, y, color);
            }
        }
        else
        {
            memset(texture->getRow(y) + x1, color, length);
        }
    }

    void Canvas::drawRectOutlined(const BoundRecti& rect, Color color)
    {
        drawVLine(rect.topLeft.y, rect.bottomRight.y, rect.topLeft.x, color);
        drawVLine(rect.topLeft.y, rect.bottomRight.y, rect.bottomRight.x, color);
        drawHLine(rect.topLeft.x, rect.bottomRight.x, rect.topLeft.y, color);
        drawHLine(rect.topLeft.x, rect.bottomRight.x, rect.bottomRight.x, color);
    }

    void Canvas::drawRectFilled(const BoundRecti& rect, Color color)
    {
    }

    void Canvas::drawString(const char* str, const RasterFont& font, Vec2i pos)
    {
        // TODO: maybe implement clipping?

        while(*str != '\0')
        {
            Color* charTexels = font.getCharTexels(*str);

            Texture charTexture(font.getCharW(), font.getCharH(), charTexels);

            blitTexture(charTexture, pos);

            pos.x += charTexture.getW();

            ++str;
        }
    }

    void Canvas::blitTexture(const Texture& tex, Vec2i pos)
    {
        for(int i = 0; i < tex.getH(); ++i)
        {
            for(int j = 0; j < tex.getW(); ++j)
            {
                texture->setTexel(
                    j + pos.x,
                    i + pos.y,
                    tex.getTexel(j, i));
            }
        }
    }
}