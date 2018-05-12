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

#include <cstdio>
#include <cstring>

#include "X_Memory.hpp"
#include "error/X_error.h"
#include "util/X_util.h"
#include "memory/X_String.h"
#include "engine/X_init.h"

unsigned char* Hunk::memoryStart;
unsigned char* Hunk::memoryEnd;

unsigned char* Hunk::highMark;
unsigned char* Hunk::lowMark;

Zone::Block* Zone::rover;

void Hunk::init(int size)
{
    memoryStart = (unsigned char *)malloc(size);
    if(!memoryStart)
    {
        x_system_error("Failed to init hunk of size %d - not enough mem", size);
    }

    memoryEnd = memoryStart + size;
    
    lowMark = memoryStart;
    highMark = memoryEnd;
}

void Hunk::cleanup()
{
    free(memoryStart);
}

void* Hunk::allocLow(int size, const char* name)
{
    // Align to multiple of 16
    size = nearestPowerOf2(size + sizeof(Header), 16);

    Header* header = (Header*)lowMark;

    lowMark += size;
    if(lowMark >= highMark)
    {
        x_system_error("Can't alloc %d bytes for %s in allocLow", size, name);
    }

    Cache::freeBelow(lowMark);

    x_strncpy(header->name, name, 8);

    header->size = size;
    header->sentinel = SENTINEL;

    return header + 1;
}

void* Hunk::allocHigh(int size, const char* name)
{
    // Align to multiple of 16
    size = nearestPowerOf2(size + sizeof(Header), 16);

    Header* header = (Header*)highMark - sizeof(Header);

    highMark -= size;
    if(highMark <= lowMark)
    {
        x_system_error("Can't alloc %d bytes for %s in allocHigh", size, name);
    }

    Cache::freeAbove(highMark);

    x_strncpy(header->name, name, 8);

    header->size = size;
    header->sentinel = SENTINEL;

    return highMark;
}

void Hunk::print()
{
    unsigned char* ptr = memoryStart;

    while(ptr < lowMark)
    {
        Header* header = (Header*)ptr;
        printf("%.8s\t\t%d\t\t%X\n", header->name, header->size, header->sentinel);

        ptr += header->size;
    }

    printf("-----------------------\n");

    printHighHunk(memoryEnd);
}

void Hunk::printHighHunk(unsigned char* ptr)
{
    if(ptr <= highMark)
        return;

    Header* header = (Header*)ptr - sizeof(Header);
    printHighHunk(ptr - header->size);

    printf("%.8s\t\t%d\t\t%X\n", header->name, header->size, header->sentinel);
}

void Cache::freeBelow(void* ptr)
{
}

void Cache::freeAbove(void* ptr)
{
}

void* Zone::tryAllocChunk(int size)
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

void* Zone::allocChunk(int size)
{
    void* chunk = tryAllocChunk(size);

    if(!chunk)
    {
        x_system_error("Can't allocate %d bytes in zone", size);
    }

    return chunk;
}

void Zone::free(void* mem)
{
    if(!mem)
    {
        return;
    }

    Block* block = blockFromChunk(mem);

    block = block->tryMergeWithAdjacentBlocks();
    block->setIsFree(true);
}

enum ReallocSource
{
    SOURCE_ZONE = 0,
    SOURCE_STACK = 1,
    SOURCE_HEAP = 2
};

void* Zone::reallocChunk(void* ptr, int newSize)
{
    if(!ptr)
    {
        return allocChunk(newSize);
    }

    // Request to shrink the block size
    Block* block = blockFromChunk(ptr);
    if(newSize <= block->getSize())
    {
        auto split = block->trySplit(newSize);
        if(!split)
        {
            return ptr;
        }

        split->tryMergeWithAdjacentBlocks();

        return ptr;
    }

    int blockSize = block->getSize();

    // Try and avoid a copy by merging with the next block (if it's free and is big enough)
    if(block->next->isFree() && block->next->getSize() + blockSize >= newSize)
    {
        block = block->mergeWithNext(block->next);
        block->trySplit(newSize);

        return ptr;
    }

    // Not ideal, but try and merge with the previous block. This will still cause a copy
    // but we won't have to do another allocation, a copy, and then a free (which may fail)
    if(block->prev->isFree() && block->prev->getSize() + blockSize >= newSize)
    {
        auto newBlock = block->prev->mergeWithNext(block);
        void* newChunk = blockFromChunk(newBlock);

        memmove(newChunk, ptr, blockSize);

        auto split = block->trySplit(newSize);
        if(split)
        {
            split->tryMergeWithAdjacentBlocks();
        }

        return ptr;
    }

    // Worst case, have to do a new allocation
    ReallocSource source;
    void* newChunk = tryAllocChunk(newSize);
    void* tempBuf = nullptr;
    void* chunk = nullptr;

    // Find a temporary place to put the data
    if(newChunk)
    {
        chunk = ptr;
        source = SOURCE_ZONE;    
    }
    else if(newSize <= 1024)
    {
        tempBuf = alloca(newSize);
        source = SOURCE_STACK;
    }
    else
    {
        // Now we're really getting desperate...
        tempBuf = malloc(newSize);
        source = SOURCE_HEAP;
    }

    if(source != SOURCE_ZONE)
    {
        memcpy(tempBuf, ptr, block->getSize());
        Zone::free(ptr);
        newChunk = tryAllocChunk(newSize);
        chunk = tempBuf;
    }

    if(chunk && newChunk)
    {
        memmove(newChunk, chunk, blockSize);
    }

    if(source == SOURCE_HEAP)
    {
        free(tempBuf);
    }

    if(!newChunk)
    {
        x_system_error("Failed to realloc mem in zone of size %d\n", newSize);
    }

    return newChunk;
}

Zone::Block* Zone::Block::tryMergeWithAdjacentBlocks()
{
    Block* block = this;

    if(prev->isFree())
    {
        block = block->prev->mergeWithNext(block);
    }

    if(block->next->isFree())
    {
        block = block->mergeWithNext(block->next);
    }

    return block;
}

Zone::Block* Zone::Block::mergeWithNext(Block* next)
{
    if(this == next)
    {
        return this;
    }

    if(Zone::rover == next)
    {
        Zone::rover = this;
    }

    next->unlink();

    setSize(getSize() + next->getSize());

    return this;
}

Zone::Block* Zone::Block::trySplit(int size)
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

void Zone::init(int size)
{
    unsigned char* mem = (unsigned char*)Hunk::allocLow(size, "zone");
    Block* block = (Block*)(mem + sizeof(Block));

    block->next = block;
    block->prev = block;
    block->setSize(size);
    block->setIsFree(true);

    rover = block;
}

void Zone::print()
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

void MemoryManager::init(ConfigurationFile& config)
{
    auto section = config.getSection("memory");

    int hunkSize = section->getPositiveInt("hunkSize", false, 8 * 1024 * 1024);
    Hunk::init(hunkSize);
    
    int zoneSize = section->getPositiveInt("zoneSize", false, 48 * 1024);
    Zone::init(zoneSize);
}

void MemoryManager::cleanup()
{
    // Only the hunk needs to be cleaned up because the other allocators allocate
    // space from the hunk
    Hunk::cleanup();
}

