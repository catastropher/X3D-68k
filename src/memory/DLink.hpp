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

template<typename T>
struct DLink
{
    DLink()
        : next(nullptr),
        prev(nullptr)
    {

    }
    
    void insertAfterThis(T* link)
    {
        link->prev = static_cast<T*>(this);
        link->next = next;
        
        if(next != nullptr)
            next->prev = link;
        
        next = link;
    }
    
    void insertBeforeThis(T* link)
    {
        link->next = static_cast<T*>(this);
        link->prev = prev;
        
        if(prev != nullptr)
            prev->next = link;
        
        prev = link;
    }
    
    void insertMeBetween(T* prev, T* next)
    {
        this->next = next;
        this->prev = prev;
        
        if(next != nullptr)
            next->prev = static_cast<T*>(this);
        
        if(prev != nullptr)
            prev->next = static_cast<T*>(this);
    }
    
    void unlink()
    {
        if(prev)
            prev->next = next;
        
        if(next)
            next->prev = prev;
        
        next = nullptr;
        prev = nullptr;
    }
    
    T* next;
    T* prev;
};

