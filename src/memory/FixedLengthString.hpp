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

#include <cstdarg>
#include <cstdio>
#include <cstring>

template<int MaxLength>
class FixedLengthString
{
public:
    FixedLengthString()
    {
        buf[0] = '\0';
    }

    FixedLengthString(const char* str)
    {
        copyFromCString(str);
    }

    void format(const char* formatStr, va_list list)
    {
        vsnprintf(buf, MaxLength, formatStr, list);
    }

    void vformat(const char* formatStr, ...)
    {
        va_list list;
        va_start(list, formatStr);

        format(formatStr, list);

        va_end(list);
    }

    template<int OtherMaxLength>
    FixedLengthString<MaxLength>& operator=(const FixedLengthString<OtherMaxLength>& str)
    {
        copyFromCString(str.buf);

        return *this;
    }

    FixedLengthString& operator=(const char* str)
    {
        copyFromCString(str);

        return *this;
    }

    char operator[](int index) const
    {
        return buf[index];
    }

    template<int OtherMaxLength>
    bool operator==(const FixedLengthString<OtherMaxLength>& str)
    {
        return strcmp(buf, str.buf) == 0;
    }

    bool operator==(const char* str)
    {
        return strcmp(buf, str) == 0;
    }

    template<int OtherMaxLength>
    bool operator!=(const FixedLengthString<OtherMaxLength>& str)
    {
        return !(*this == str);
    }

    bool operator!=(const char* str)
    {
        return !(*this == str);
    }

    constexpr int maxLength() const
    {
        return MaxLength;
    }

    int length() const
    {
        return strlen(buf);
    }

    const char* c_str() const
    {
        return buf;
    }

private:
    void copyFromCString(const char* str)
    {
        strncpy(buf, str, MaxLength);

        // Always append a null terminator just to be safe
        buf[MaxLength - 1] = '\0';
    }

    char buf[MaxLength];
};

