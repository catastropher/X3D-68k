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

#include <cstring>

#include "error/Error.hpp"

#define X_EDICT_MAX_NAME_LENGTH 32
#define X_EDICT_MAX_ATTRIBUTES 64

struct BrushModelId
{
    BrushModelId()
        : id(-1)
    {

    }

    BrushModelId(int id_)
        : id(id_)
    {

    }

    int id;
};

struct X_EdictAttribute
{
    char name[X_EDICT_MAX_NAME_LENGTH];
    char* value;
};

template<typename T>
void edictParseAttribute(const char* value, T& outValue);

struct X_Edict
{
    template<typename T>
    bool getValueOrDefault(const char* name, T& outValue, const T& defaultValue) const
    {

        if(getValue(name, outValue))
        {
            return true;
        }

        outValue = defaultValue;

        return false;
    }

    bool getValueOrDefault(const char* name, char* outValue, const char* defaultValue) const
    {
        if(!getValue(name, outValue))
        {
            strcpy(outValue, defaultValue);

            return false;
        }

        return true;
    }

    template<typename T>
    void getRequiredValue(const char* name, T& outValue) const
    {
        if(!getValue(name, outValue))
        {
            x_system_error("Missing required value: %s.", name);
        }
    }

    template<typename T>
    bool getValue(const char *name, T &outValue) const
    {
        X_EdictAttribute* attribute = getAttribute(name);

        if(attribute == nullptr)
        {
            return false;
        }
        else
        {
            edictParseAttribute(attribute->value, outValue);

            return true;
        }
    }

    void print();

    X_EdictAttribute* getAttribute(const char* name) const;
    bool hasAttribute(const char* name) const;

    mutable X_EdictAttribute attributes[X_EDICT_MAX_ATTRIBUTES];
    int totalAttributes;
};

