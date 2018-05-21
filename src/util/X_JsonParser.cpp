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

        case '{':
            return parseObject();

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
    JsonValue* stringValue = Json::newValue(JSON_STRING);
    parseStringLiteral(stringValue->stringValue);

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
            x_system_error("Unterminated '['");
        }
        
        if(*next == ']')
        {
            ++next;
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

JsonValue* JsonParser::parseObject()
{
    ++next;

    JsonValue* objectValue = Json::newValue(JSON_OBJECT);

    do
    {
        skipWhitespace();

        if(*next == '\0')
        {
            x_system_error("Unterminated '{'");
        }
        else if(*next == '}')
        {
            ++next;
            break;
        }

        KeyValuePair<String, JsonValue*> pair;

        parseStringLiteral(pair.key);
        skipWhitespace();
        expect(':');
        skipWhitespace();
        pair.value = parseValue();

        objectValue->object.pairs.push_back(std::move(pair));

        if(*next == ',')
        {
            ++next;
        }
    } while(true);

    return objectValue;
}

void JsonParser::parseStringLiteral(String& dest)
{
    expect('"');

    do
    {
        if(*next == '\0')
        {
            x_system_error("Unterminated '\"'");
        }
        else if(*next == '"')
        {
            ++next;
            break;
        }
        else if(*next == '\\')
        {
            ++next;
            char c = '\\';

            switch(*next)
            {
                case '\\':      c = '\\'; break;
                case 'b':       c = '\b'; break;
                case 'n':       c = '\n'; break;
                case '"':       c = '"'; break;
                case 'f':       c = '\f'; break;
                case 'r':       c = '\r'; break;
                case 't':       c = '\t'; break;
                default:
                    x_system_error("Unknown escape character \\%c", *next);
            }

            dest.push_back(c);
            ++next;
        }
        else
        {
            dest.push_back(*next++);
        }
    } while(true);
}

void JsonParser::skipWhitespace()
{
    while(*next == ' ' || *next == '\t' || *next == '\n')
    {
        ++next;
    }
}

void JsonParser::expect(char c)
{
    if(*next != c)
    {
        x_system_error("Expected '%c', found '%c'", c, *next);
    }

    ++next;
}

