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

#include <algorithm>

#include "FixedSizeArray.hpp"

// TODO: this can be made a lot more efficient by e.g. keeping the array sorted

template<typename T, int MaxElements>
class Set
{
public:
    void add(const T& val)
    {
        if(find(val) == end())
        {
            elements.pushBack(val);
        }
    }

    void remove(const T& val)
    {
        T* newEnd = std::remove(begin(), end(), val);
        elements.setEnd(newEnd);
    }

    bool contains(const T& val) const
    {
        return find(val) != end();
    }

    T* begin()
    {
        return elements.begin();
    }

    T* end()
    {
        return elements.end();
    }

    const T* begin() const
    {
        return elements.begin();
    }

    const T* end() const
    {
        return elements.end();
    }

    const T* find(const T& val) const
    {
        return std::find(begin(), end(), val);
    }

private:
    FixedLengthArray<T, MaxElements> elements;
};

