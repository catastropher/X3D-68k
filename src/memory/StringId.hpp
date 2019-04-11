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

#ifndef NDEBUG
    #define DEBUG_STRINGID
#endif

class StringId
{
public:
    constexpr StringId()
        : key(0),
#ifdef DEBUG_STRINGID
        originalString("<default>")
#endif
    {

    }

    constexpr StringId(const char* str)
        : key(crc32Constexpr(str)),
#ifdef DEBUG_STRINGID
        originalString(str)
#endif
    {

    }

    constexpr StringId(unsigned int key_)
        : key(key_),
#ifdef DEBUG_STRINGID
        originalString("<from key>")
#endif
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

    void toString(char* dest)
    {
#ifdef DEBUG_STRINGID
        sprintf(dest, "%s (%d)\n", originalString, key);
#else
        sprintf(dest, "%d\n", key);
#endif
    }

    void print(const char* description) const
    {
#ifdef DEBUG_STRINGID
        printf("%s: %s (%d)\n", description, originalString, key);
#else
        printf("%d: %d\n", description, key);
#endif
    }

    static StringId fromString(const char* str)
    {
#ifdef DEBUG_STRINGID
        return StringId(crc32(str), str);
#else
        return StringId(crc32(str));
#endif
    }

    unsigned int key;

private:
#ifdef DEBUG_STRINGID
    constexpr StringId(unsigned int key_, const char* str)
        : key(key_),
        originalString(str)
    {

    }

    const char* originalString;
#endif
};

constexpr StringId operator ""_sid(const char* str, size_t len)
{
    return StringId(str);
}

#undef DEBUG_STRINGID

