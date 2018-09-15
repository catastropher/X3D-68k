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

#include <cstddef>

#include "X_Lru.hpp"
#include "X_DLinkBaseAux.hpp"

namespace X3D
{
    class LinearAllocator;
    class CacheHandle;

    // The DLinkBase<> is used for the LRU, the DLinkBaseAux<> is used for the chain
    struct CacheBlock : LruNodeBase<CacheBlock>, DLinkBaseAux<CacheBlock>
    {
        void* getMem()
        {
            return (unsigned char *)this + sizeof(CacheBlock);
        }

        int size;
        int id;
        CacheHandle* handle;
    };

    class CacheHandle
    {
    public:
        CacheHandle()
            : mem(nullptr)
        {
            
        }

    private:
        CacheBlock* getCacheBlock()
        {
            return (CacheBlock*)((unsigned char*)mem - sizeof(CacheBlock));
        }

        void* mem;

        friend class Cache;
    };


    class Cache
    {
    public:
        Cache(LinearAllocator& linearAllocator_);
        
        void alloc(int size, CacheHandle& dest, int id);
        bool tryAlloc(int size, CacheHandle& dest, bool allowAllocOnBottom, int id);

        void free(CacheHandle& handle);
        void freeAllMatchingId(int id, int mask = 0x7FFFFFFF);
        void flush();
        void freeBelowLowMark(unsigned char* lowMark);
        void freeAboveHighMark(unsigned char* highMark);

        void* getCachedData(CacheHandle& handle);

        void printBlocks();
        void printSize();

    private:
        void free(CacheBlock* block);

        LinearAllocator& linearAllocator;
        LruList<CacheBlock> cacheLru;
        CacheBlock cacheHead;
        CacheBlock cacheTail;
    };
}


