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

#include "X_alloc.h"
#include "error/X_error.h"

template<typename T>
class ArenaAllocator
{
public:
    ArenaAllocator(int maxAllocs, const char* name_)
    {
        arenaStart = (T*)malloc(sizeof(T) * maxAllocs);
        arenaEnd = arenaStart + maxAllocs;
        nextAlloc = arenaStart;
        name = name_;
    }
    
    T* alloc()
    {
        if(nextAlloc == arenaEnd)
            x_system_error("Arena allocator full: %s", name);
        
        return nextAlloc++;
    }
    
    void freeAll()
    {
        nextAlloc = arenaStart;
    }
    
    void freeLast()
    {
        --nextAlloc;
    }
    
    T* begin() const
    {
        return arenaStart;
    }
    
    T* end() const
    {
        return nextAlloc;
    }
    
    T* allocationEnd() const
    {
        return arenaEnd;
    }
    
    int totalAllocs() const
    {
        return nextAlloc - arenaStart;
    }
    
    int maxAllocs() const
    {
        return arenaEnd - arenaStart;
    }
    
    ~ArenaAllocator()
    {
        free(arenaStart);
    }
    
private:
    T* arenaStart;
    T* arenaEnd;
    T* nextAlloc;
    const char* name;
};

