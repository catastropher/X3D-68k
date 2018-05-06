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

#include "X_Memory.hpp"
#include "error/X_error.h"
#include "util/X_util.h"
#include "memory/X_String.h"

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

void* Zone::allocChunk(int size)
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
            x_system_error("Can't allocate %d bytes in zone", size);
        }

    } while(true);

    block->setIsFree(false);
    
    int sizeLeft = block->getSize() - size;

    if(sizeLeft >= Block::MIN_SIZE)
    {
        Block* split = (Block*)((unsigned char*)block + size);

        split->setIsFree(true);
        split->setSize(sizeLeft);

        block->setSize(size);
        block->insertAfterThis(split);
    }

    rover = block->next;

    return (void*)((unsigned char*)block + sizeof(Block));
}

void Zone::free(void* mem)
{
    Block* block = (Block*)((unsigned char*)mem - sizeof(Block));

    if(block->prev->isFree())
    {
        block = block->prev->mergeWithNext(block);
    }

    if(block->next->isFree())
    {
        block = block->mergeWithNext(block->next);
    }

    block->setIsFree(true);
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
        printf("Block size: %d, free: %d\n", block->getSize(), block->isFree());

        block = block->next;
    } while(block != start);

    printf("============================\n");
}

