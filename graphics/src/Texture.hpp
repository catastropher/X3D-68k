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

#include <X3D-System.hpp>
#include <X3D-math.hpp>

#include "filesystem/X_FilePath.hpp"
#include "Color.hpp"

namespace X3D
{
    // Note: a Texture does not own the memory of the texels!
    class Texture
    {
    public:
        Texture()
        {

        }

        Texture(int w_, int h_, Color* texels_)
            : w(w_),
            h(h_),
            texels(texels_)
        {

        }

        int getW() const
        {
            return w;
        }

        int getH() const
        {
            return h;
        }

        Color* getTexels()
        {
            return texels;
        }

        int totalTexels() const
        {
            return w * h;
        }

        Color* getRow(int row)
        {
            return texels + row * w;
        }

        void setTexel(int x, int y, Color color)
        {
            texels[texelIndex(x, y)] = color;
        }

        void setTexel(Vec2i pos, Color color)
        {
            texels[texelIndex(pos.x, pos.y)] = color;
        }

        Color getTexel(int x, int y) const
        {
            return texels[texelIndex(x, y)];
        }

        void setDimensions(int newW, int newH)
        {
            w = newW;
            h = newH;
        }

        void setTexels(Color* texels_, int newW, int newH)
        {
            texels = texels_;
            w = newW;
            h = newH;
        }

    protected:
        constexpr int texelIndex(int x, int y) const
        {
            return y * w + x;
        }

        int w;
        int h;
        Color* texels;
    };

    class ManagedTexture : public Texture
    {
    public:
        ManagedTexture(AllocationSource source_)
            : Texture(0, 0, nullptr),
            source(source_)
        {

        }

        ManagedTexture(int w_, int h_, AllocationSource source_ = AllocationSource::zone);

        void resize(int newW, int newH);
        void loadFromFile(FilePath& path);

        ~ManagedTexture();

    private:
        void cleanup();
        void setSize(int newW, int newH);

        AllocationSource source;
    };
}

