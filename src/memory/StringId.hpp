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

#include <cstdio>
#include "Crc32.hpp"

struct StringId
{
    constexpr StringId()
        : key(0)
    {

    }

    constexpr StringId(const char* str)
        : key(crc32Constexpr(str))
    {

    }

    constexpr StringId(unsigned int key_)
        : key(key_)
    {

    }

    constexpr operator unsigned int() const
    {
        return key;
    }

    constexpr bool operator==(const StringId& rhs) const
    {
        return key == rhs.key;
    }

    static StringId fromString(const char* str)
    {
        return StringId(crc32(str));
    }

    unsigned int key;
};

constexpr StringId operator ""_sid(const char* str, size_t len)
{
    return StringId(str);
}

