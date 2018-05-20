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

#include "X_Json.hpp"
#include "error/X_error.h"
#include "X_JsonParser.hpp"

JsonValue* Json::parse(const char* str)
{
    JsonParser parser(str);

    return parser.parse();
}

JsonValue::~JsonValue()
{
    switch(type)
    {
        case JSON_OBJECT:
            object.~JsonObject();
            break;

        case JSON_ARRAY:
            array.~JsonArray();
            break;

        case JSON_STRING:
            stringValue.~String();
            break;

        default:
            break;
    }

    type = JSON_NULL;
}

template<>
int fromJson<int>(JsonValue* value)
{
    switch(value->type)
    {
        case JSON_INT:
            return value->iValue;
        
        case JSON_FP:
            return value->fpValue.toInt();

        default:
            x_system_error("Value is not an int\n");
    }
}

static void stringifyValue(JsonValue* value, bool pretty, int indent, String& dest);

static void stringifyArray(JsonValue* value, bool pretty, int indent, String& dest)
{
    dest.push_back('[');

    auto& arr = value->array.values;
    for(int i = 0; i < (int)arr.size(); ++i)
    {
        if(pretty)
        {
            dest.push_back('\n');
        }

        stringifyValue(arr[i], pretty, indent + 1, dest);

        if(i != (int)arr.size() - 1)
        {
            dest.push_back(',');
        }
    }

    if(pretty)
    {
        dest.push_back('\n');

        for(int i = 0; i < indent; ++i)
        {
            dest.push_back('\t');
        }
    }

    dest.push_back(']');
}

static void stringifyValue(JsonValue* value, bool pretty, int indent, String& dest)
{
    if(pretty)
    {
        for(int i = 0; i < indent; ++i)
        {
            dest.push_back('\t');
        }
    }

    switch(value->type)
    {
        case JSON_STRING:
            dest.push_back('"');
            dest += value->stringValue;
            dest.push_back('"');
            break;

        case JSON_ARRAY:
            stringifyArray(value, pretty, indent, dest);
            break;

        default:
            x_system_error("Unknown JsonType: %d\n", value->type);
    }
}

String Json::stringify(JsonValue* value, bool pretty)
{
    String res;
    stringifyValue(value, pretty, 0, res);

    return res;
}





