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

#include "memory/X_String.h"
#include "dev/X_Console.h"

#pragma once

struct ConsoleVariable
{
    ConsoleVariable(const char* name_) : name(name_), next(nullptr) { }

    virtual bool trySetValue(const char* value) = 0;
    virtual void toXString(XString& dest) const = 0;
    virtual const char* getTypeName() const = 0;

    const char* name;
    ConsoleVariable* next;
};

template<>
void X_Console::addVariable(const char* name, int& var);

