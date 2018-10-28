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

#include "Texture.hpp"

namespace X3D
{
    ManagedTexture::ManagedTexture(int w_, int h_, AllocationSource source_)
        : source(source_)
    {
        setSize(w_, h_);
    }

    void ManagedTexture::resize(int newW, int newH)
    {
        // Is there a reason to copy over the old data? This will currently lose the old image
        cleanup();
        setSize(newW, newH);
    }

    ManagedTexture::~ManagedTexture()
    {
        cleanup();
    }

    void ManagedTexture::cleanup()
    {
        if(texels != nullptr)
        {
            auto memoryManager = ServiceLocator::get<MemoryManager>();
            memoryManager->free(texels, source);
            texels = nullptr;
        }
    }

    void ManagedTexture::setSize(int newW, int newH)
    {
        w = newW;
        h = newH;

        auto memoryManager = ServiceLocator::get<MemoryManager>();
        texels = (Color*)memoryManager->alloc(totalTexels() * sizeof(Color), source);
    }
}

