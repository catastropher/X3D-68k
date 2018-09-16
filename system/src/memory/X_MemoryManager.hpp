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

#include "config/X_MemoryManagerConfig.hpp"
#include "X_SystemAllocator.hpp"
#include "X_LinearAllocator.hpp"
#include "X_ZoneAllocator.hpp"
#include "X_Cache.hpp"
#include "X_AllocationSource.hpp"
#include "X_Array.hpp"

namespace X3D
{
    class MemoryManager
    {
    public:
        MemoryManager(MemoryManagerConfig& config);

        template<typename T>
        T* allocLinearVolatile(int count, const char* name = "unknown")
        {
            return (T*)linearAllocator.allocLow(count * sizeof(T), name);
        }

        template<typename T>
        T* zoneAlloc(int count = 1)
        {
            return zoneAllocator.alloc<T>(count);
        }

        void zoneFree(void* mem)
        {
            zoneAllocator.free(mem);
        }

        Cache& getCache()
        {
            return cache;
        }

        template<typename T>
        void alloc(int count, AllocationSource source, Array<T>& dest)
        {
            dest.size = sizeof(T) * count;
            dest.data = (T*)alloc(dest.size, source);
        }

        void* alloc(int size, AllocationSource source);

        ~MemoryManager();
        
        SystemAllocator systemAllocator;
        LinearAllocator linearAllocator;
        Cache cache;
        ZoneAllocator zoneAllocator;
    };
};

