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

namespace X3D
{
    template<typename T>
    class Array
    {
    public:
        Array(T* data_, int size_)
            : data(data_),
            size(size_)
        {

        }

        Array()
            : data(nullptr),
            size(0)
        {

        }

        void set(T* data_, int size_)
        {
            data = data_;
            size = size_;
        }

        int getSize() const
        {
            return size;
        }

        T& operator[](int index) const
        {
            return data[index];
        }

        T* begin() const
        {
            return data;
        }

        T* end() const
        {
            return data + size;
        }
        
        T* data;
        int size;
    };

    template<typename T, int Size>
    class FixedSizeArray : public Array<T>
    {
    public:
        FixedSizeArray()
            : Array<T>(elem, Size)
        {

        }

        T elem[Size];
    };
}

