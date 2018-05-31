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

#include "X_MemoryAllocator.hpp"
#include "X_UnrolledList.hpp"

template<typename T>
class UnrolledListAllocator
{
public:
    using Node = UnrolledListNode<T>;

    UnrolledListAllocator(MemoryAllocator& allocator_, int nodeCapacity_)
        : allocator(allocator_),
        nodeCapacity(nodeCapacity_) { }

    T* alloc()
    {
        if(!head || head->count == head->capacity)
        {
            Node* newNode = Node::alloc(allocator, nodeCapacity);
            newNode->next = head;
            head = newNode;
        }

        return head->elem + head->count++;
    }

private:
    Node* head;
    MemoryAllocator allocator;
    int nodeCapacity;
};

