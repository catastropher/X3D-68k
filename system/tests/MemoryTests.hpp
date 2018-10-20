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

#include <X3D/X3D-System.hpp>

using namespace X3D;

#define TEST_FAILED(args...) Log::error("Test %s failed: ", __FUNCTION__, #args)

class MemoryTests
{
public:
    MemoryTests()
    {
        SystemConfig config = 
        {
            .memoryManager = 
            {
                .linearAllocatorSize = 50000,
                .zoneAllocatorSize = ZONE_SIZE
            }
        };

        System::init(config);
        memoryManager = ServiceLocator::get<MemoryManager>();
    }

    void run()
    {
        //allocCache();
        //bootOldOutOfCache();
        //AllocTooMuchFromZone();
        //FreeMemNotFromZone();
    }

    void allocCache()
    {
        CacheHandle handle[10];
        memoryManager->cache->tryAlloc(100, handle[0], true, 0);
        memoryManager->cache->printBlocks();

        printf("\n");
        memoryManager->cache->tryAlloc(100, handle[1], true, 1);
        memoryManager->cache->printBlocks();

        printf("\n");
        memoryManager->cache->tryAlloc(100, handle[2], true, 2);
        memoryManager->cache->printBlocks();
        memoryManager->cache->free(handle[1]);
        memoryManager->cache->printBlocks();

        printf("\n");
        memoryManager->cache->tryAlloc(50, handle[3], true, 3);
        memoryManager->cache->printBlocks();

        printf("\n");
        memoryManager->cache->tryAlloc(50, handle[4], true, 4);

        printf("\n");
        memoryManager->cache->tryAlloc(8, handle[5], true, 5);

        memoryManager->cache->tryAlloc(5, handle[6], true, 6);

        memoryManager->cache->printBlocks();
    }

    void bootOldOutOfCache()
    {
        CacheHandle handle[50];

        for(int i = 0; i < 10; ++i)
        {
            memoryManager->getCache().alloc(100, handle[i], i);
            memoryManager->getCache().printBlocks();
            printf("=====================\n");
        }

        memoryManager->linearAllocator->allocHigh(400, "blah");
    }

    void AllocTooMuchFromZone()
    {
        bool pass = false;

        try
        {
            memoryManager->zoneAlloc<unsigned char>(ZONE_SIZE + 1);
        }
        catch(const OutOfMemoryException& e)
        {
            pass = true;
        }
        catch(...)
        {
        }

        if(!pass)
        {
            TEST_FAILED();
        }
    }

    void FreeMemNotFromZone()
    {
        bool pass = false;

        try
        {
            char mem[128];
            memoryManager->zoneFree(mem);
        }
        catch(const RuntimeException& e)
        {
            pass = true;
        }
        catch(...)
        {

        }

        if(!pass)
        {
            TEST_FAILED();
        }
    }

    ~MemoryTests()
    {
        System::cleanup();
    }

private:


    static const int ZONE_SIZE = 20000;

    MemoryManager* memoryManager;
};

