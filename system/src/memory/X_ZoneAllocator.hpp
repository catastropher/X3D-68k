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

#include "X_DLinkBase.hpp"

namespace X3D
{
    class LinearAllocator;

    class ZoneAllocator
    {
    public:
        ZoneAllocator(int size, LinearAllocator& linearAllocator);

        template<typename T>
        T* alloc(int count = 1)
        {
            return (T*)allocChunk(count * sizeof(T));
        }

        template<typename T>
        T* realloc(T* ptr, int newCount)
        {
            return (T*)reallocChunk(ptr, newCount * sizeof(T));
        }

        void init(int size);
        void free(void* ptr);

        void print();

    private:
        struct Block : DLinkBase<Block>
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

            Block* mergeWithNext(Block* next, Block*& rover);
            Block* trySplit(int size);
            Block* tryMergeWithAdjacentBlocks(Block*& rover);

            static const int MIN_SIZE = 64;
            static const unsigned int SIZE_MASK = 0x00FFFFFF;
            static const int FREE_BIT = 31;
            static const unsigned int FREE_MASK = (1 << FREE_BIT);

            unsigned int flags;
        };

        bool memoryInRegion(void* mem)
        {
            return mem >= regionStart && mem < regionEnd;
        }

        static Block* blockFromChunk(void* ptr)
        {
            return (Block *)((unsigned char*)ptr - sizeof(Block));
        }

        static void* chunkFromBlock(Block* block)
        {
            return (void *)((unsigned char*)block + sizeof(Block));
        }

        void* tryAllocChunk(int size);
        void* allocChunk(int size);
        void* reallocChunk(void* ptr, int newSize);

        Block* rover;
        void* regionStart;
        void* regionEnd;
    };
};

