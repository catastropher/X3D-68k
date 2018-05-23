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

#include <new>

#include "memory/X_Array.hpp"
#include "memory/X_KeyValuePair.hpp"
#include "memory/X_String.h"
#include "math/X_fix.h"

enum JsonType
{
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_INT,
    JSON_FP,
    JSON_STRING,
    JSON_BOOL,
    JSON_NULL
};

struct JsonValue;

struct JsonObject
{
    void addProperty(const char* name, JsonValue* value)
    {
        KeyValuePair<String, JsonValue*> pair;
        pair.key = name;
        pair.value = value;

        pairs.push_back(std::move(pair));
    }

    Array<KeyValuePair<String, JsonValue*>> pairs;
};

struct JsonArray
{
    Array<JsonValue*> values;
};

struct JsonValue
{
    JsonValue() { }

    ~JsonValue();

    JsonType type;

    union
    {
        JsonObject object;
        JsonArray array;
        int iValue;
        fp fpValue;
        bool boolValue;
        String stringValue;
    };

    JsonValue& operator[](int index);
    JsonValue& operator[](const char* name);

    static JsonValue nullValue;
    static JsonValue trueValue;
    static JsonValue falseValue;

    friend class Json;
};

template<typename T>
inline T fromJson(JsonValue* value)
{
    return T::fromJson(value);
}

template<>
int fromJson<int>(JsonValue* value);

class Json
{
public:
    static JsonValue* newValue(JsonType type)
    {
        auto value = Zone::alloc<JsonValue>();
        value->type = type;

        switch(type)
        {
            case JSON_STRING:
                new (&value->stringValue) String;
                break;

            case JSON_ARRAY:
                new (&value->array) JsonObject;
                break;

            case JSON_OBJECT:
                new (&value->object) JsonObject;
                break;

            default:
                break;
        }

        return value;
    }

    static JsonValue* newString(const char* begin, const char* end)
    {
        auto value = Zone::alloc<JsonValue>();
        value->type = JSON_STRING;

        new (&value->stringValue) String(begin, end);

        return value;
    }

    static String stringify(JsonValue* value, bool pretty);

    static JsonValue* parse(const char* str);

    friend class MemoryManager;

private:
    static void init()
    {
        JsonValue::nullValue.type = JSON_NULL;

        JsonValue::trueValue.type = JSON_BOOL;
        JsonValue::trueValue.boolValue = true;

        JsonValue::falseValue.type = JSON_BOOL;
        JsonValue::falseValue.boolValue = false;
    }
    
};

template<typename T>
inline JsonValue* toJson(const T& value)
{
    return value.toJson();
}

template<>
inline JsonValue* toJson(const int& value)
{
    auto jsonValue = Json::newValue(JSON_INT);
    jsonValue->iValue = value;

    return jsonValue;
}

template<>
inline JsonValue* toJson(const float& value)
{
    auto jsonValue = Json::newValue(JSON_FP);
    jsonValue->fpValue = fp::fromFloat(value);

    return jsonValue;
}

template<>
inline JsonValue* toJson(const fp& value)
{
    auto jsonValue = Json::newValue(JSON_FP);
    jsonValue->fpValue = value;

    return jsonValue;
}

template<>
inline JsonValue* toJson(const char* const& str)
{
    auto jsonValue = Json::newValue(JSON_STRING);
    jsonValue->stringValue = str;

    return jsonValue;
}

template<>
inline JsonValue* toJson(const String& str)
{
    auto jsonValue = Json::newValue(JSON_STRING);
    jsonValue->stringValue = str;

    return jsonValue;
}

template<typename T>
inline JsonValue* toJson(T* arr, int size)
{
    auto jsonValue = Json::newValue(JSON_ARRAY);
    
    for(int i = 0; i < size; ++i)
    {
        jsonValue->array.values.push_back(toJson(arr[i]));
    }

    return jsonValue;
}

template<typename T>
inline JsonValue* toJson(Array<T>& arr)
{
    auto jsonValue = Json::newValue(JSON_ARRAY);
    
    for(int i = 0; i < arr.size(); ++i)
    {
        jsonValue->array.values.push_back(toJson(arr[i]));
    }

    return jsonValue;
}

