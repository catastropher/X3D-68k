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

#include "memory/X_String.h"
#include "math/X_fix.h"

typedef enum X_ConsoleVarType
{
    X_CONSOLEVAR_INT,
    X_CONSOLEVAR_FLOAT,
    X_CONSOLEVAR_FP16X16,
    X_CONSOLEVAR_STRING
} X_ConsoleVarType;

typedef struct X_ConsoleVar
{
    const char* name;
    X_String stringValue;
    X_ConsoleVarType type;
    _Bool saveToConfig;
    
    union {
        int intValue;
        float floatValue;
        x_fp16x16 fp16x16Value;
    };
    
    struct X_ConsoleVar* next;
} X_ConsoleVar;

typedef struct X_Console
{
    X_ConsoleVar* consoleVarsHead;
} X_Console;


void x_consolevar_init(X_ConsoleVar* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig);
void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue);

void x_console_init(X_Console* console);
_Bool x_console_var_exists(X_Console* console, const char* name);
void x_console_printf(X_Console* console, const char* format, ...);
void x_console_register_var(X_Console* console, X_ConsoleVar* var);

