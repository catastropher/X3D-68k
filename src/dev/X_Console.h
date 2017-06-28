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
#include "geo/X_Vec2.h"
#include "render/X_Font.h"
#include "render/X_Screen.h"
#include "system/X_Keys.h"
#include "system/X_Time.h"

typedef enum X_ConsoleVarType
{
    X_CONSOLEVAR_INT,
    X_CONSOLEVAR_FLOAT,
    X_CONSOLEVAR_FP16X16,
    X_CONSOLEVAR_STRING,
    X_CONSOLEVAR_BOOL
} X_ConsoleVarType;

#define X_CONSOLE_INPUT_BUF_SIZE 512

struct X_EngineContext;

typedef struct X_ConsoleVar
{
    const char* name;
    X_String assignedValueString;
    X_ConsoleVarType type;
    _Bool saveToConfig;
    
    union {
        int* intPtr;
        float* floatPtr;
        x_fp16x16* fp16x16Ptr;
        _Bool* boolPtr;
        X_String* stringPtr;
        void* voidPtr;
    };
    
    struct X_ConsoleVar* next;
} X_ConsoleVar;

typedef struct X_Console
{
    X_ConsoleVar* consoleVarsHead;
    X_Vec2 cursor;
    X_Vec2 size;
    const X_Font* font;
    struct X_EngineContext* engineContext;
    _Bool isOpen;
    char* text;
    char input[X_CONSOLE_INPUT_BUF_SIZE + 2];
    int inputPos;
    X_Color backgroundColor;
    
    _Bool showCursor;
    X_Time lastCursorBlink;
    X_Key lastKeyPressed;
} X_Console;


void x_console_register_var(X_Console* console, X_ConsoleVar* consoleVar, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig);
void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue);

void x_console_init(X_Console* console, struct X_EngineContext* engineContext, X_Font* font);
void x_console_cleanup(X_Console* console);

void x_console_clear(X_Console* console);
_Bool x_console_var_exists(X_Console* console, const char* name);
void x_console_print(X_Console* console, const char* str);
void x_console_printf(X_Console* console, const char* format, ...);
void x_console_render(X_Console* console);

void x_console_send_key(X_Console* console, X_Key key);

void x_console_execute_cmd(X_Console* console, const char* str);

static inline void x_console_open(X_Console* console)
{
    console->isOpen = 1;
}

static inline void x_console_close(X_Console* console)
{
    console->isOpen = 0;
}

static inline _Bool x_console_is_open(const X_Console* console)
{
    return console->isOpen;
}

