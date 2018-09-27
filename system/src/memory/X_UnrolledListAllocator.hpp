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

#include "X_MemoryManager.hpp"

namespace X3D
{
    template<typename T>
    struct UnrolledListNode
    {
        UnrolledListNode* next;
        T data[];
    };

    template<typename T>
    class UnrolledListIterator
    {
        using Node = UnrolledListNode<T>;

    public:
        UnrolledListIterator(Node* node_, int index_, int countPerNode_)
            : node(node_),
            index(index_),
            countPerNode(countPerNode_)
        {

        }

        bool operator==(const UnrolledListIterator& it) const
        {
            return node == it.node && index == it.index;
        }

        bool operator!=(const UnrolledListIterator& it) const
        {
            return !(*this == it);
        }

        T& operator*()
        {
            return node->data[index];
        }

        UnrolledListIterator& operator++()
        {
            if(++index == countPerNode)
            {
                node = node->next;
                index = 0;
            }

            return *this;
        }

    private:
        Node* node;
        int index;
        int countPerNode;
    };

    template<typename T>
    class UnrolledListAllocator
    {
        using Node = UnrolledListNode<T>;

    public:
        UnrolledListAllocator(int countPerNode_, MemoryManager& memoryManager_, AllocationSource source = AllocationSource::zone)
            : head(nullptr),
            tail(nullptr),
            countPerNode(countPerNode_),
            countInLastNode(countPerNode_),
            totalAllocs(0),
            memoryManager(memoryManager_),
            allocSource(source)
        {

        }

        T* alloc()
        {
            ++totalAllocs;

            Node* lastNode = countInLastNode < countPerNode
                ? tail
                : appendNode();

            return lastNode->data + countInLastNode++;
        }

        void freeAll()
        {
            Node* node = head;

            while(node)
            {
                Node* next = node->next;
                memoryManager.free(node, allocSource);

                node = next;
            }

            head = nullptr;
            tail = nullptr;
            countInLastNode = countPerNode;
            totalAllocs = 0;
        }

        int getTotalAllocs()
        {
            return totalAllocs;
        }

        UnrolledListIterator<T> begin() const
        {
            return UnrolledListIterator<T>(head, 0, countPerNode);
        }

        UnrolledListIterator<T> end() const
        {
            // Guarantee we move to the next node (null) if the last one is actually full
            return ++UnrolledListIterator<T>(tail, countInLastNode - 1, countPerNode);
        }

        ~UnrolledListAllocator()
        {
            freeAll();
        }

    private:
        Node* appendNode()
        {
            int nodeSize = sizeof(Node) + sizeof(T) * countPerNode;
            Node* node = (Node*)memoryManager.alloc(nodeSize, allocSource);

            node->next = tail;
            tail = node;

            if(!head)
            {
                head = node;
            }

            countInLastNode = 0;

            return node;
        }

        Node* head;
        Node* tail;
        int countPerNode;
        int countInLastNode;
        int totalAllocs;
        MemoryManager& memoryManager;
        AllocationSource allocSource;
    };
}

