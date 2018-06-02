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

#include "X_FilePath.hpp"

char* FilePath::end()
{
    return path + strlen(path);
}

char* FilePath::startOfLastSegment()
{
    char* ptr = end();

    do
    {
        --ptr;
    } while(ptr > path && *ptr != '/');

    return ptr;
}

#include <cstdio>

FilePath& FilePath::appendSegment(const char* segment)
{
    if(segment[0] == '\0')
    {
        strcpy(path, segment);
        return *this;
    }

    // Don't double add the separating slash
    if(segment[0] == '/')
    {
        ++segment;
    }

    char* ptr = end() - 1;
    if(*ptr != '/')
    {
        ++ptr;
        *ptr++ = '/';
    }

    strcpy(ptr, segment);

    return *this;
}

FilePath& FilePath::removeLastSegment()
{
    char* lastSegment = startOfLastSegment();
    *lastSegment = '\0';

    return *this;
}

