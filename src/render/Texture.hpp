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

#include "memory/Allocator.hpp"
#include "geo/Vec2.hpp"
#include "math/FixedPoint.hpp"

////////////////////////////////////////////////////////////////////////////////
/// An 8-bit color palette index.
////////////////////////////////////////////////////////////////////////////////
typedef unsigned char X_Color;

struct Font;

enum TextureFlags
{
    TEXTURE_ALLOCED = 1
};

////////////////////////////////////////////////////////////////////////////////
/// A texture, which is basically a 2D array of texels ("texture elements")
class X_Texture
{
public:
    X_Texture()
        : w(0), h(0), texels(nullptr), flags(0)
    { }

    X_Texture(int w_, int h_)
        : w(w_), h(h_), flags(TEXTURE_ALLOCED)
    {
        texels = xalloc<X_Color>(w * h);
    }

    X_Texture(int w_, int h_, X_Color* texels_)
        : w(w_), h(h_), texels(texels_), flags(0)
    { }

    int getW() const{ return w; }
    int getH() const { return h; }
    int totalTexels() const { return w * h; }
    X_Color* getTexels() const { return texels; }

    void setTexels(X_Color* texels_)
    {
        // FIXME: should free mem
        texels = texels_;
        flags &= (~TEXTURE_ALLOCED);
    }

    int texelIndex(Vec2i pos) const
        { return pos.y * w + pos.x; }

    X_Color* getRow(int row) const
        { return texels + row * w; }

    void setTexel(Vec2i pos, X_Color color)
    {
        texels[texelIndex(pos)] = color;
    }

    X_Color getTexel(Vec2i pos) const
        { return texels[texelIndex(pos)]; }

    void resize(int newW, int newH)
    {
        w = newW;
        h = newH;

        if(flags & TEXTURE_ALLOCED)
            xfree(texels);

        texels = xalloc<X_Color>(totalTexels());
        flags |= TEXTURE_ALLOCED;
    }

    void setDimensions(int w_, int h_)
    {
        w = w_;
        h = h_;
    }

    bool saveToFile(const char* fileName);
    bool loadFromFile(const char* fileName);

    void clampVec2i(Vec2i& v);
    void drawLine(Vec2i start, Vec2i end, X_Color color);
    void drawLineShaded(Vec2i start, Vec2i end, X_Color color, fp startIntensity, fp endIntensity, X_Color* colorTable);
    void blit(const X_Texture& tex, Vec2i pos);
    void drawChar(int c, const Font& font, Vec2i pos);
    void drawStr(const char* str, const Font& font, Vec2i pos);
    void fillRect(Vec2i topLeft, Vec2i bottomRight, X_Color color);
    void fill(X_Color color);
    void drawDecal(X_Texture& decal, Vec2i pos, Vec2fp& uOrientation, Vec2fp& vOrientation, X_Color transparency);

    ~X_Texture()
    {
        if(flags & TEXTURE_ALLOCED)
            xfree(texels);

        // TODO: is it worth it to set these to default?
        w = 0;
        h = 0;
        texels = nullptr;
    }

private:
    int w;              ///< Width of the texture
    int h;              ///< Height of the texture
    X_Color* texels;    ///< Texels
    int flags;
};

