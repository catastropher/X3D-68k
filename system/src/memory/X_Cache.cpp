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

#include "X_Cache.hpp"
#include "X_LinearAllocator.hpp"
#include "error/X_OutOfMemoryException.hpp"
#include "log/X_Log.hpp"
#include "service/X_ServiceLocator.hpp"

namespace X3D
{
    // FIXME: move to math library
    static inline int nearestPowerOf2(int x, int powerOf2)
    {
        return (x + powerOf2 - 1) & ~(powerOf2 - 1);
    }


    void Cache::init()
    {
        linearAllocator = ServiceLocator::get<LinearAllocator>();

        DLinkBaseAux<CacheBlock>::initLink(&cacheHead, &cacheTail);

        Log::info("Init cache");
        printSize();
    }

    void Cache::alloc(int size, CacheHandle& dest, int id)
    {
        // This is a really dumb way to do this, and makes allocations O(N^2)
        // TODO: smarter strategy for kicking out blocks
        while(!tryAlloc(size, dest, true, id))
        {
            LOG_DEBUG("Need to boot a block out of cache\n");
            CacheBlock* lru = cacheLru.getLeastRecentlyUsed();

            LOG_DEBUG("LRU pointer: %p", lru);
            LOG_DEBUG("Least recently used block: %d\n", lru->id);

            this->free(lru);
        }
    }

    bool Cache::tryAlloc(int size, CacheHandle& dest, bool allowAllocOnBottom, int id)
    {
        size = nearestPowerOf2(size +  sizeof(CacheBlock), 4);  // TODO: use CPU alignment

        LOG_DEBUG("Try alloc %d bytes in cache for block %d", size, id);

        unsigned char* lowMark = linearAllocator->getLowMark();
        unsigned char* highMark = linearAllocator->getHighMark();

        CacheBlock* blockToInsertBefore = cacheHead.nextAux;
        CacheBlock* newBlock = (CacheBlock*)lowMark;

        bool cacheIsEmpty = cacheHead.nextAux == &cacheTail;
        if(allowAllocOnBottom && cacheIsEmpty)
        {
            LOG_DEBUG("Alloc from bottom of cache");

            if(highMark - lowMark < size)
            {
                LOG_DEBUG("Can't fit in linear allocator. Max cache size: %d", (int)(highMark - lowMark));
                throw OutOfMemoryException(size, "cache");
            }
        }
        else
        {
            LOG_DEBUG("Searching for enough room...");

            do
            {
                int freeSize = (unsigned char*)blockToInsertBefore - (unsigned char*)newBlock;

                LOG_DEBUG("\t\tFound free chunk of size %d before block %d", freeSize, blockToInsertBefore->id);

                if(freeSize >= size)
                {
                    LOG_DEBUG("\t\t\t-Big enough");
                    break;
                }

                newBlock = (CacheBlock*)((unsigned char*)blockToInsertBefore + blockToInsertBefore->size);
                blockToInsertBefore = blockToInsertBefore->nextAux;
            } while(blockToInsertBefore != &cacheTail);
        }

        if(blockToInsertBefore == &cacheTail)
        {
            int freeSize = highMark - (unsigned char*)newBlock;

            LOG_DEBUG("Search high end of memory. Has %d free", freeSize);

            LOG_DEBUG("Previous block: %d\n", blockToInsertBefore->prevAux->id);

            if(freeSize < size)
            {
                return false;
            }
        }

        LOG_DEBUG("Found block!");

        newBlock->insertMeBetweenAux(blockToInsertBefore->prevAux, blockToInsertBefore);
        newBlock->size = size;

        newBlock->next = nullptr;
        newBlock->prev = nullptr;
        cacheLru.markAsMostRecentlyUsed(newBlock);

        newBlock->id = id;
        newBlock->handle = &dest;

        dest.mem = newBlock->getMem();

        return true;
    }

    void Cache::free(CacheHandle& handle)
    {
        CacheBlock* block = handle.getCacheBlock();
        this->free(block);
    }

    void Cache::free(CacheBlock* block)
    {
        LOG_DEBUG("Free block %d", block->id);

        block->unlinkAux();
        cacheLru.remove(block);

        block->handle->mem = nullptr;
    }

    void* Cache::getCachedData(CacheHandle& handle)
    {
        if(handle.mem != nullptr)
        {
            cacheLru.markAsMostRecentlyUsed(handle.getCacheBlock());
        }

        return handle.mem;
    }

    void Cache::freeAllMatchingId(int id, int mask)
    {
        CacheBlock* block = cacheHead.nextAux;

        while(block != &cacheTail)
        {
            CacheBlock* next = block->nextAux;

            if((block->id & mask) == id)
            {
                this->free(block);
            }

            block = next;
        }
    }

    void Cache::flush()
    {
        freeAllMatchingId(0, 0);
    }

    void Cache::freeBelowLowMark(unsigned char* lowMark)
    {
        CacheBlock* block = cacheHead.nextAux;

        while(block != &cacheTail)
        {
            CacheBlock* next = block->nextAux;

            unsigned char* blockStart = (unsigned char*)block;
            if(blockStart < lowMark)
            {
                this->free(block);
            }
            else
            {
                break;
            }

            block = next;
        }
    }

    void Cache::freeAboveHighMark(unsigned char* highMark)
    {
        CacheBlock* block = cacheTail.prevAux;

        while(block != &cacheHead)
        {
            CacheBlock* prev = block->prevAux;

            unsigned char* blockEnd = (unsigned char*)block + block->size;
            if(blockEnd > highMark)
            {
                this->free(block);
            }

            block = prev;
        }
    }

    void Cache::printBlocks()
    {
        CacheBlock* block = cacheHead.nextAux;

        printf("head");

        unsigned char* lowMark = linearAllocator->getLowMark();

        while(block != &cacheTail)
        {
            printf(" -> %d(%d)", block->id, (int)((unsigned char *)block - lowMark));
            block = block->nextAux;
        }

        printf(" -> tail\n\nLRU: ");

        CacheBlock* lru = cacheLru.getLeastRecentlyUsed();

        while(lru)
        {
            printf("%d ", lru->id);
            lru = lru->next;
        }

        printf("\n");
    }

    void Cache::printSize()
    {
        unsigned char* lowMark = linearAllocator->getLowMark();
        unsigned char* highMark = linearAllocator->getHighMark();

        int sizeInKilobytes = nearestPowerOf2(highMark - lowMark, 1024) / 1024;
        Log::info("Current cache size: %dk", sizeInKilobytes);
    }
}

