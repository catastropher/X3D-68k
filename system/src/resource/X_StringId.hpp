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

#include "memory/X_Crc32.hpp"

#define DEBUG_STRING_ID

namespace X3D
{
    struct StringId
    {
        constexpr StringId(const char* str)
            : key(crc32Constexpr(str))
#ifdef DEBUG_STRING_ID
            ,str(str)
#endif
        {
        }

        StringId(unsigned int key_)
            : key(key_)
        {

        }

        const char* toString()
        {
#ifdef DEBUG_STRING_ID
            return str;
#else
            static char str[16];
            sprintf(str, "%u", key);
#endif
        }

        static StringId fromString(const char* str)
        {
#ifdef DEBUG_STRING_ID
            return StringId(str, crc32(str));
#else
            return StringId(crc32(str));
#endif
        }

        bool operator==(const StringId& id)
        {
            return key == id.key;
        }

        unsigned int key;
        
#ifdef DEBUG_STRING_ID
        const char* str;
    private:
        StringId(const char* str_, unsigned int key_)
            : key(key_),
            str(str_)
        {

        }
#endif


    };

    constexpr StringId operator ""_sid(const char* str, size_t len)
    {
        return StringId(str);
    }
}


