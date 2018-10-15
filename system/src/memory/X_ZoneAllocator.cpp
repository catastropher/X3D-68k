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

#include "X_ZoneAllocator.hpp"
#include "log/X_Log.hpp"
#include "X_LinearAllocator.hpp"
#include "error/X_OutOfMemoryException.hpp"
#include "error/X_RuntimeException.hpp"

namespace X3D
{
    // FIXME: move to math library
    static inline int nearestPowerOf2(int x, int powerOf2)
    {
        return (x + powerOf2 - 1) & ~(powerOf2 - 1);
    }

    ZoneAllocator::ZoneAllocator(int size, LinearAllocator& linearAllocator)
    {
        Log::info("Init zone allocator (size = %d)", size);

        regionStart = linearAllocator.allocLow(size, "zone");
        regionEnd = (unsigned char*)regionStart + size;

        Block* block = (Block*)((unsigned char*)regionStart + sizeof(Block));

        block->next = block;
        block->prev = block;
        block->setSize(size);
        block->setIsFree(true);

        rover = block;
    }

    void* ZoneAllocator::tryAllocChunk(int size)
    {
        size = nearestPowerOf2(size + sizeof(Block), 8);

        Block* start = rover;
        Block* block = rover;

        do
        {
            if(block->isFree() && block->getSize() >= size)
            {
                break;
            }

            block = block->next;

            if(block == start)
            {
                return nullptr;
            }

        } while(true);

        block->setIsFree(false);
        block->trySplit(size);

        rover = block->next;

        return chunkFromBlock(block);
    }

    void* ZoneAllocator::allocChunk(int size)
    {
        void* chunk = tryAllocChunk(size);

        if(!chunk)
        {
            throw OutOfMemoryException(size, "zone");
        }

        return chunk;
    }

    void ZoneAllocator::free(void* mem)
    {
        if(!mem)
        {
            return;
        }

        Block* block = blockFromChunk(mem);

        if(!memoryInRegion(mem))
        {
            throw RuntimeException("Trying to free memory not from zone");
        }

        if(block->isFree())
        {
            throw RuntimeException("Trying to free zone block that is already free");
        }

        block = block->tryMergeWithAdjacentBlocks(rover);
        block->setIsFree(true);
    }
    
    ZoneAllocator::Block* ZoneAllocator::Block::tryMergeWithAdjacentBlocks(Block*& rover)
    {
        Block* block = this;

        if(prev->isFree())
        {
            block = block->prev->mergeWithNext(block, rover);
        }

        if(block->next->isFree())
        {
            block = block->mergeWithNext(block->next, rover);
        }

        return block;
    }

    ZoneAllocator::Block* ZoneAllocator::Block::mergeWithNext(Block* next, Block*& rover)
    {
        if(this == next)
        {
            return this;
        }

        if(rover == next)
        {
            rover = this;
        }

        next->unlink();

        setSize(getSize() + next->getSize());

        return this;
    }

    ZoneAllocator::Block* ZoneAllocator::Block::trySplit(int size)
    {
        int sizeLeft = getSize() - size;
        if(sizeLeft < Block::MIN_SIZE)
        {
            return nullptr;
        }

        Block* split = (Block*)((unsigned char*)this + size);

        split->setIsFree(true);
        split->setSize(sizeLeft);

        setSize(size);
        insertAfterThis(split);

        return split;
    }

    void ZoneAllocator::print()
    {
        Block* start = rover;
        Block* block = start;

        printf("============zone============\n");

        do
        {
            printf("Block size: %d, free: %d, next: %lld, prev: %lld\n",
                block->getSize(),
                block->isFree(),
                (long long)block->next,
                (long long)block->prev);

            block = block->next;
        } while(block != start);

        printf("============================\n");
    }
};

