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

#include <cstdlib>

#include "X_Memory.hpp"

enum AllocatorSource
{
    ALLOC_ZONE,
    ALLOC_SYSTEM
};

class MemoryAllocator
{
public:
    MemoryAllocator(AllocatorSource source_) : source(source_) { }

    template<typename T>
    T* alloc(int count)
    {
        switch(source)
        {
            case ALLOC_ZONE:    return Zone::alloc<T>(count);
            case ALLOC_SYSTEM:  return (T*)malloc(count * sizeof(T));
        }
    }

    template<typename T>
    T* allocBlock(int blockSize)
    {
        return (T*)alloc<unsigned char>(blockSize);
    }

    template<typename T>
    void free(T* ptr)
    {
        switch(source)
        {
            case ALLOC_ZONE:    Zone::free(ptr);
            case ALLOC_SYSTEM:  free(ptr);
        }
    }

private:
    AllocatorSource source;
};

