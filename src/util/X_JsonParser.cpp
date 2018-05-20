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

#include "X_JsonParser.hpp"
#include "error/X_error.h"

JsonValue* JsonParser::parse()
{
    return parseValue();
}

JsonValue* JsonParser::parseValue()
{
    skipWhitespace();

    switch(*next)
    {
        case '"':
            return parseString();

        case '[':
            return parseArray();

        // case '{':
        //     return parseObject();

        // case 't':
        // case 'f':
        //     return parseBool();

        // case 'n':
        //     return parseNull();

        // case '0'...'9':
        //     return parseNumber();

        default:
            x_system_error("Unexpected character '%c'\n", *next);
    }
}

JsonValue* JsonParser::parseString()
{
    ++next;

    const char* start = next;

    while(*next && *next != '"')
    {
        ++next;
    }

    if(*next == '\0')
    {
        x_system_error("Unterminated '\"'");
    }

    JsonValue* stringValue = Json::newString(start, next);

    ++next;

    return stringValue;
}

JsonValue* JsonParser::parseArray()
{
    JsonValue* arr = Json::newValue(JSON_ARRAY);

    ++next;

    do
    {
        skipWhitespace();

        if(*next == '\0')
        {
            x_system_error("Unterminated ']");
        }
        
        if(*next == ']')
        {
            break;
        }

        JsonValue* arrValue = parseValue();
        arr->array.values.push_back(arrValue);

        skipWhitespace();

        if(*next == ',')
        {
            ++next;
        }
    } while(true);

    return arr;
}

void JsonParser::skipWhitespace()
{
    while(*next == ' ' || *next == '\t' || *next == '\n')
    {
        ++next;
    }
}

