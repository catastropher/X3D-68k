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

#include "geo/X_Vec3.h"
#include "math/X_convert.hpp"

class StreamReader
{
public:
    StreamReader(const char* begin_, const char* end_)
        : ptr(begin_),
        end(end_)
    {

    }

    StreamReader() { }

    StreamReader& readFrom(const char* begin_, const char* end_)
    {
        ptr = begin_;
        end = end_;

        return *this;
    }

    template<typename T>
    StreamReader& read(T& dest);

    template<typename T>
    StreamReader& read(Vec3Template<T>& v);

    template<typename From, typename To>
    StreamReader& readAs(To& dest)
    {
        From from;
        read(from);
        convert(from, dest);

        return *this;
    }

    template<typename T>
    StreamReader& readArray(T* arr, int count)
    {
        for(int i = 0; i < count; ++i)
        {
            read(arr[i]);
        }

        return *this;
    }

private:
    unsigned int next()
    {
        return *(unsigned char*)ptr++;
    }

    template<int Bytes>
    int readInt()
    {
        unsigned int res = 0;

        for(int i = 0; i < Bytes; ++i)
        {
            res |= next() << (i * 8);
        }

        return res;
    }

    const char* ptr;
    const char* end;
};

template<>
inline StreamReader& StreamReader::read(int& dest)
{
    dest = readInt<4>();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(unsigned int& dest)
{
    dest = readInt<4>();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(short& dest)
{
    dest = readInt<2>();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(unsigned short& dest)
{
    dest = readInt<2>();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(char& dest)
{
    dest = next();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(unsigned char& dest)
{
    dest = next();

    return *this;
}

template<>
inline StreamReader& StreamReader::read(float& dest)
{
    union
    {
        float f;
        int i;
    };

    i = readInt<4>();
    dest = f;

    return *this;
}

template<typename T>
inline StreamReader& StreamReader::read(Vec3Template<T>& v)
{
    read(v.x);
    read(v.y);
    read(v.z);

    return *this;
}

