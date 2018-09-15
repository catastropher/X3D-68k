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

// Same as DLinkBase<>, but has everything renamed with "aux" to allow for two
// sets of doublely linked pointers in the same object.

template<typename T>
struct DLinkBaseAux
{
    DLinkBaseAux()
        : nextAux(nullptr),
        prevAux(nullptr)
    {
        
    }
    
    void insertAfterThisAux(T* link)
    {
        link->prevAux = static_cast<T*>(this);
        link->nextAux = nextAux;
        
        if(nextAux != nullptr)
            nextAux->prevAux = link;
        
        nextAux = link;
    }
    
    void insertBeforeThisAux(T* link)
    {
        link->nextAux = static_cast<T*>(this);
        link->prevAux = prevAux;
        
        if(prevAux != nullptr)
            prevAux->nextAux = link;
        
        prevAux = link;
    }
    
    void insertMeBetweenAux(T* prevAux, T* nextAux)
    {
        this->nextAux = nextAux;
        this->prevAux = prevAux;
        
        if(nextAux != nullptr)
            nextAux->prevAux = static_cast<T*>(this);
        
        if(prevAux != nullptr)
            prevAux->nextAux = static_cast<T*>(this);
    }
    
    void unlinkAux()
    {
        if(prevAux)
            prevAux->nextAux = nextAux;
        
        if(nextAux)
            nextAux->prevAux = prevAux;
        
        nextAux = nullptr;
        prevAux = nullptr;
    }

    static void initLink(T* head, T* tail)
    {
        head->nextAux = tail;
        head->prevAux = nullptr;

        tail->prevAux = head;
        tail->nextAux = nullptr;
    }
    
    T* nextAux;
    T* prevAux;
};

