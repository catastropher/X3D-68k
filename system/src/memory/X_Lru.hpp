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
    template<typename T>
    struct LruNodeBase : DLinkBase<T>
    {
    };

    template<typename T>
    class LruList
    {
    public:
        LruList()
            : head(nullptr),
            tail(nullptr)
        {

        }

        LruList(T* nodeListStart, T* nodeListEnd)
            : head(nodeListStart),
            tail(nodeListEnd - 1)
        {
            linkArrayOfDoublyLinkedNodes(nodeListStart, nodeListEnd);
        }

        void markAsLeastRecentlyUsed(T* node)
        {
            node->unlink();

            if(head)
            {
                node->insertBeforeThis(head);
            }

            head = node;
        }

        void markAsMostRecentlyUsed(T* node)
        {
            node->unlink();

            if(tail)
            {
                node->insertAfterThis(tail);
            }

            tail = node;
        }

        void remove(T* node)
        {
            if(node == head)
            {
                head = node->next;
            }

            if(node == tail)
            {
                tail = node->prev;
            }

            node->unlink();
        }

        T* getLeastRecentlyUsed()
        {
            return head;
        }

        T* getMostRecentlyUsed()
        {
            return tail;
        }

    private:
        T* head;    // Least recently used
        T* tail;    // Most recently used
    };
}

