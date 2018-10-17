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

#include "X_LinearAllocator.hpp"
#include "X_SystemAllocator.hpp"
#include "error/X_OutOfMemoryException.hpp"
#include "X_Cache.hpp"
#include "X_MemoryManager.hpp"

#include "log/X_Log.hpp"

namespace X3D
{
    // FIXME: move to math library
    static inline int nearestPowerOf2(int x, int powerOf2)
    {
        return (x + powerOf2 - 1) & ~(powerOf2 - 1);
    }

    void LinearAllocator::init(MemoryManagerConfig& config)
    {
        sysAllocator = ServiceLocator::get<SystemAllocator>();
        cache = ServiceLocator::get<Cache>();

        Log::info("Init linear allocator (size = %d)", config.linearAllocatorSize);

        memoryStart = (unsigned char *)sysAllocator->alloc(config.linearAllocatorSize);
        memoryEnd = memoryStart + config.linearAllocatorSize;
        
        lowMark = memoryStart;
        highMark = memoryEnd;
    }

    void LinearAllocator::cleanup()
    {
        Log::info("Cleanup linear allocator");

        sysAllocator->free(memoryStart);    
    }

    void* LinearAllocator::allocLow(int size, const char* name)
    {
        // Align to multiple of 16
        size = nearestPowerOf2(size + sizeof(Header), 16);

        Header* header = (Header*)lowMark;

        lowMark += size;
        if(lowMark >= highMark)
        {
            Log::error("LinearAllocator: can't allocLow %d bytes for %s", size, name);
            throw OutOfMemoryException(size, "linear");
        }

        cache->freeBelowLowMark(lowMark);

        header->name = name;
        header->size = size;
        header->sentinel = SENTINEL;

        Log::info("AllocLow %d bytes for %s", size, name);

        return header + 1;
    }

    void* LinearAllocator::allocHigh(int size, const char* name)
    {
        // Align to multiple of 16
        size = nearestPowerOf2(size + sizeof(Header), 16);

        Header* header = (Header*)highMark - sizeof(Header);

        highMark -= size;
        if(highMark <= lowMark)
        {
            Log::error("LinearAllocator: can't allocHigh %d bytes for %s", size, name);
            throw OutOfMemoryException(size, "linear");
        }

        cache->freeAboveHighMark(highMark);

        header->name = name;
        header->size = size;
        header->sentinel = SENTINEL;

        Log::info("AllocHigh %d bytes for %s", size, name);

        return highMark;
    }

    void LinearAllocator::print()
    {
        unsigned char* ptr = memoryStart;

        while(ptr < lowMark)
        {
            Header* header = (Header*)ptr;
            printf("%.8s\t\t%d\t\t%X\n", header->name.c_str(), header->size, header->sentinel);

            ptr += header->size;
        }

        printf("-----------------------\n");

        printHighHunk(memoryEnd);
    }

    void LinearAllocator::printHighHunk(unsigned char* ptr)
    {
        if(ptr <= highMark)
        {
            return;
        }

        Header* header = (Header*)ptr - sizeof(Header);
        printHighHunk(ptr - header->size);

        printf("%.8s\t\t%d\t\t%X\n", header->name.c_str(), header->size, header->sentinel);
    }
};

