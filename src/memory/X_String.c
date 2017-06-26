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

#include <string.h>

#include "X_String.h"
#include "X_alloc.h"

void x_string_init(X_String* str, const char* initialValue)
{
    str->data = x_malloc(strlen(initialValue) + 1);
    strcpy(str->data, initialValue);
}

void x_string_cleanup(X_String* str)
{
    if(str->data)
        x_free(str->data);
    
    str->data = NULL;
}

X_String* x_string_assign(X_String* str, const char* value)
{
    str->data = x_realloc(str->data, strlen(value) + 1);
    strcpy(str->data, value);
    return str;
}

X_String* x_string_concat(X_String* strToAppendTo, const X_String* strToAppend)
{
    strToAppendTo->data = x_realloc(strToAppendTo->data, strlen(strToAppendTo->data) + strlen(strToAppend->data) + 1);
    strcat(strToAppendTo->data, strToAppend->data);
    return strToAppendTo;
}

