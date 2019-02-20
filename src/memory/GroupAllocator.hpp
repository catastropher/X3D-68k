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

struct GroupAllocation
{
    int offset;
    void** dest;
};

// Performs several smaller allocations as one giant allocation to reduce fragmentation
class GroupAllocator
{
public:
    GroupAllocator()
        : totalAllocations(0),
        totalSize(0)
    {

    }

    static const int MAX_ALLOCATIONS = 128;

    template<typename T>
    void scheduleAlloc(T*& outPointer, int count = 1)
    {
        allocations[totalAllocations].offset = totalSize;
        allocations[totalAllocations].dest = (void**)&outPointer;

        totalSize += sizeof(T) * count;
    }

    void allocAll();

private:
    GroupAllocation allocations[MAX_ALLOCATIONS];
    int totalAllocations;
    int totalSize;
};

