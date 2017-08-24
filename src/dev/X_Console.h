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
} X_ConsoleVar;


typedef void (*X_ConsoleCmdHandler)(struct X_EngineContext* context, int argc, char* argv[]);

typedef struct X_ConsoleCmd
{
    const char* name;
    X_ConsoleCmdHandler handler;    
} X_ConsoleCmd;


typedef enum X_ConsoleOpenState
{
    X_CONSOLE_STATE_OPEN,
    X_CONSOLE_STATE_CLOSED,
    X_CONSOLE_STATE_OPENING,
    X_CONSOLE_STATE_CLOSING
} X_ConsoleOpenState;

typedef struct X_Console
{
    X_ConsoleVar* consoleVars;
    int totalConsoleVars;
    
    X_ConsoleCmd* consoleCmds;
    int totalConsoleCmds;
    
    X_Vec2 cursor;
    X_Vec2 size;
    const X_Font* font;
    struct X_EngineContext* engineContext;
    char* text;
    char input[X_CONSOLE_INPUT_BUF_SIZE + 2];
    int inputPos;
    
    _Bool showCursor;
    X_Time lastCursorBlink;
    X_Key lastKeyPressed;
    
    X_ConsoleOpenState openState;
    int renderYOffset;
    X_Time consoleToggleTime;    
} X_Console;

void x_console_open(X_Console* console);
void x_console_close(X_Console* console);

X_ConsoleCmd* x_console_get_cmd(X_Console* console, const char* cmdName);
_Bool x_console_cmd_exists(X_Console* console, const char* cmdName);
void x_console_register_cmd(X_Console* console, const char* name, X_ConsoleCmdHandler handler);

void x_console_register_var(X_Console* console, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig);
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

static inline _Bool x_console_is_open(const X_Console* console)
{
    return console->openState != X_CONSOLE_STATE_CLOSED;
}


