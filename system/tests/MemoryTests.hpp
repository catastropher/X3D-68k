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
                .linearAllocatorSize = 1000,
                .zoneAllocatorSize = ZONE_SIZE
            }
        };

        auto& instance = System::init(config);
        memoryManager = &instance.getMemoryManager();
    }

    void run()
    {
        AllocTooMuchFromZone();
        FreeMemNotFromZone();
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

private:


    static const int ZONE_SIZE = 500;

    MemoryManager* memoryManager;
};

