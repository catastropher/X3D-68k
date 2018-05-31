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

#include "dev/console/X_ConsoleVariableInt.hpp"

const char* ConsoleVariableInt::getTypeName() const
{
    return "int";
}

bool ConsoleVariableInt::trySetValue(const char* value)
{
    int val = 0;
    while(*value)
    {
        if(*value < '0' || *value > '9')
        {
            return false;
        }

        val = val * 10 + *value - '0';

        ++value;
    }

    return true;
}

void ConsoleVariableInt::toString(String& dest) const
{
    char buf[32];
    sprintf(buf, "%d", var);

    dest = buf;
}

