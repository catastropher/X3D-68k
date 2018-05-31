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

#include "X_DLink.hpp"

struct MemoryConfig;

class Cache;

class Hunk
{
public:
    static void* allocHigh(int size, const char* name);
    static void* allocLow(int size, const char* name);

    static void* getHighMark();
    static void freeToHighMark(void* highMark);

    static void* getLowMark();
    static void freeToLowMark(void* lowMark);

    static void init(int size);
    static void cleanup();

    static void print();

private:
    struct Header
    {
        int size;
        unsigned int sentinel;
        char name[8];
    };

    static void printHighHunk(unsigned char* ptr);

    static const unsigned int SENTINEL = 0xEC53DB01;

    static unsigned char* memoryStart;
    static unsigned char* memoryEnd;

    static unsigned char* highMark;
    static unsigned char* lowMark;

    friend class Cache;    
};

class CacheEntry
{

};

class Cache
{
public:
    static void init();
    static void* alloc(int size);
    static void free(void* mem);
    static void flush();

private:
    struct Block : DLink<Block>
    {
        int size;
        DLink<Block> lru;
        unsigned char data[];
    };

    static void freeBelow(void* ptr);
    static void freeAbove(void* ptr);

    friend class Hunk;
};

class Zone
{
public:
    template<typename T>
    static T* alloc(int count = 1)
    {
        return (T*)allocChunk(count * sizeof(T));
    }

    template<typename T>
    static T* realloc(T* ptr, int newCount)
    {
        return (T*)reallocChunk(ptr, newCount * sizeof(T));
    }

    static void init(int size);
    static void free(void* ptr);

    static void print();

private:
    struct Block : DLink<Block>
    {
        int getSize() const
        {
            return flags & SIZE_MASK;
        }

        void setSize(int size)
        {
            flags = (flags & ~SIZE_MASK) | size;
        }

        bool isFree() const
        {
            return (flags & FREE_MASK) != 0;
        }

        void setIsFree(bool free)
        {
            flags = (flags & ~FREE_MASK) | ((int)free << FREE_BIT);
        }

        Block* mergeWithNext(Block* next);
        Block* trySplit(int size);
        Block* tryMergeWithAdjacentBlocks();

        static const int MIN_SIZE = 64;
        static const unsigned int SIZE_MASK = 0x00FFFFFF;
        static const int FREE_BIT = 31;
        static const unsigned int FREE_MASK = (1 << FREE_BIT);

        unsigned int flags;
    };

    static Block* blockFromChunk(void* ptr)
    {
        return (Block *)((unsigned char*)ptr - sizeof(Block));
    }

    static void* chunkFromBlock(Block* block)
    {
        return (void *)((unsigned char*)block + sizeof(Block));
    }

    static Block* rover;

    static void* tryAllocChunk(int size);
    static void* allocChunk(int size);
    static void* reallocChunk(void* ptr, int newSize);
};

class ConfigurationFile;

class MemoryManager
{
public:
    static void init(int hunkSize, int zoneSize);
    static void cleanup();
};

