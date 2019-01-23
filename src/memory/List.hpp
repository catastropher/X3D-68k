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

#include <new>
#include <functional>

#include "memory/Memory.hpp"

// A link of a singly linked list
template<typename T>
struct SLink : T
{
    SLink<T>* next;
};

template<typename T>
struct List
{
    using MatchFunction = std::function<bool(T&)>;
    using Link = SLink<T>;

    List() : head(nullptr) { }

    Link* findFirst(MatchFunction matches)
    {
        for(auto ptr = head; ptr != nullptr; ptr = ptr->next)
        {
            if(matches(*ptr))
            {
                return ptr;
            }
        }

        return nullptr;
    }

    void deleteWhere(MatchFunction matches)
    {
        Link* prev = nullptr;

        for(auto ptr = head; ptr != nullptr;)
        {
            if(matches(*ptr))
            {
                auto next = ptr->next;

                if(prev)
                {
                    prev->next = ptr->next;
                }
                else
                {
                    head = ptr->next;
                }

                destroyNode(ptr);

                ptr = next;
            }
            else
            {
                prev = ptr;
            }
        }
    }

    void prepend(Link* link)
    {
        link->next = head;
        head = link;
    }

    void append(Link* link)
    {
        link->next = nullptr;

        if(head == nullptr)
        {
            head = link;
            return;
        }

        Link* ptr = head;
        while(ptr->next != nullptr)
        {
            ptr = ptr->next;
        }

        ptr->next = link;
    }

    static Link* createNode()
    {
        return Zone::alloc<Link>();
    }

    static void destroyNode(Link* link)
    {
        Zone::free(link);
    }

    Link* head;
};


