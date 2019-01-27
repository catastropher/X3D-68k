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


#include <cstring>

#include "geo/Vec3.hpp"
#include "EntityDictionary.hpp"

const char* parseEdict(const char* nextEntry, char* valueData, X_Edict* dest)
{
    if(*nextEntry != '{')
        return nullptr;

    nextEntry += 2;         // Skip quote and newline

    dest->totalAttributes = 0;

    while(*nextEntry && *nextEntry != '}')
    {
        X_EdictAttribute* att = dest->attributes + dest->totalAttributes++;
        char* name = att->name;

        ++nextEntry;        // Skip quote

        do
        {
            *name++ = *nextEntry++;
        } while(*nextEntry != '"');

        *name = '\0';

        nextEntry += 3;     // Skip [" "]

        att->value = valueData;

        do
        {
            *valueData++ = *nextEntry++;
        } while(*nextEntry != '"');

        nextEntry += 2;     // Skip quote and newline
        *valueData++ = '\0';
    }

    return nextEntry + 2;   // Skip '}' and newline
}

