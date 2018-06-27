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
            return parseXString();

        case '[':
            return parseArray();

        case '{':
            return parseObject();

        case 't':
            return parseTrue();

        case 'f':
            return parseFalse();

        case 'n':
            return parseNull();

        case '0'...'9':
            return parseNumber();

        default:
            x_system_error("Unexpected character '%c'\n", *next);
    }
}

JsonValue* JsonParser::parseXString()
{
    JsonValue* stringValue = Json::newValue(JSON_STRING);
    parseXStringLiteral(stringValue->stringValue);

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

        KeyValuePair<XString, JsonValue*> pair;

        parseXStringLiteral(pair.key);
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

JsonValue* JsonParser::parseNumber()
{
    bool isFloat = false;
    char buf[128];
    char* bufptr = buf;

    do
    {
        if(*next == '.')
        {
            isFloat = true;
        }
        else if(*next < '0' || *next > '9')
        {
            break;
        }

        *bufptr++ = *next++;
    } while(true);

    JsonValue* value;

    if(isFloat)
    {
        value = Json::newValue(JSON_FP);
        value->fpValue = fp::fromFloat(atof(buf));
    }
    else
    {
        value = Json::newValue(JSON_INT);
        value->iValue = atoi(buf);
    }

    return value;
}

JsonValue* JsonParser::parseTrue()
{
    expectWord("true");

    return &JsonValue::trueValue;
}

JsonValue* JsonParser::parseFalse()
{
    expectWord("false");

    JsonValue* value = Json::newValue(JSON_BOOL);
    value->boolValue = false;

    return &JsonValue::falseValue;
}

JsonValue* JsonParser::parseNull()
{
    expectWord("null");

    return &JsonValue::nullValue;
}

void JsonParser::parseXStringLiteral(XString& dest)
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

void JsonParser::expectWord(const char* word)
{
    const char* ptr = word;

    while(*ptr)
    {
        if(*next == '\0' || *next++ != *ptr++)
        {
            x_system_error("Expected \"%s\"", word);
        }
    }
}

