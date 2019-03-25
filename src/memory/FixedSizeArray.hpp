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

#include "Array.hpp"
#include "error/Error.hpp"

template<typename T, int Length>
struct FixedLengthArray
{
public:
    FixedLengthArray()
        : count(0)
    {

    }

    int getCapacity() const
    {
        return Length;
    }

    void toArray(Array<T>& outArray) const
    {
        outArray.elem = data;
        outArray.count = count;
    }

    void pushBack(const T& value)
    {
        x_assert(count + 1 < Length, "Fixed length array buffer overflow");

        data[count++] = value;
    }

    void popBack()
    {
        x_assert(count - 1 >= 0, "Fixed length array buffer underflow");

        data[--count].~T();
    }

    T& operator[](int index)
    {
        x_assert(index >= 0 && index < count, "Array index out of bounds");

        return data[index];
    }

    const T& operator[](int index) const
    {
        x_assert(index >= 0 && index < count, "Array index out of bounds");

        return data[index];
    }

    T& peekFront() const
    {
        return (*this)[0];
    }

    T& peekBack() const
    {
        return (*this)[count - 1];
    }

    T* begin()
    {
        return &data[0];
    }

    T* end()
    {
        return &data[count];
    }

    const T* begin() const
    {
        return &data[0];
    }

    const T* end() const
    {
        return &data[count];
    }

    void setEnd(T* newEnd)
    {
        count = newEnd - &data[0];
    }

    int size() const
    {
        return count;
    }

private:
    T data[Length];
    int count;
};