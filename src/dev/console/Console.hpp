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

#include "memory/String.h"
#include "math/FixedPoint.hpp"
#include "geo/Vec2.hpp"
#include "geo/Vec3.hpp"
#include "render/Font.hpp"
#include "render/Screen.hpp"
#include "system/Keys.hpp"
#include "system/Time.hpp"
#include "ConsoleRenderer.hpp"

typedef enum X_ConsoleVarType
{
    X_CONSOLEVAR_INT,
    X_CONSOLEVAR_FLOAT,
    X_CONSOLEVAR_FP16X16,
    X_CONSOLEVAR_STRING,
    X_CONSOLEVAR_BOOL,
    X_CONSOLEVAR_VEC3
} X_ConsoleVarType;

#define X_CONSOLE_INPUT_BUF_SIZE 512

struct X_EngineContext;

typedef struct X_ConsoleVar
{
    const char* name;
    X_ConsoleVarType type;
    bool saveToConfig;
    
    union
    {
        int* intPtr;
        float* floatPtr;
        x_fp16x16* fp16x16Ptr;
        bool* boolPtr;
        X_XString* stringPtr;
        void* voidPtr;
        Vec3fp* vec3Ptr;
    };    
} X_ConsoleVar;


typedef void (*X_ConsoleCmdHandler)(struct X_EngineContext* context, int argc, char* argv[]);

typedef struct X_ConsoleCmd
{
    const char* name;
    X_ConsoleCmdHandler handler;    
} X_ConsoleCmd;

#define X_CONSOLE_COMMAND_HISTORY_SIZE 10

struct ConsoleVariable;

typedef struct Console
{
    ConsoleRenderer* renderer;
    
    
    X_ConsoleVar* consoleVars;
    int totalConsoleVars;
    
    X_ConsoleCmd* consoleCmds;
    int totalConsoleCmds;
    
    Vec2 cursor;
    Vec2 size;
    const X_Font* font;
    struct X_EngineContext* engineContext;
    char* text;
    char input[X_CONSOLE_INPUT_BUF_SIZE + 2];
    int inputPos;
    X_XString commandHistory[X_CONSOLE_COMMAND_HISTORY_SIZE];
    int commandHistorySize;
    int commandHistoryPos;

    X_Key lastKeyPressed;

    template<typename T>
    void addVariable(const char* name, T& var);

    ConsoleVariable* varHead;

    const char* getLine(int lineNumber)
    {
        return text + lineNumber * (size.x + 1);
    }
    
    bool isOpen() const
    {
        return renderer->isVisible();
    }

private:

} Console;

void x_console_open(Console* console);
void x_console_close(Console* console);
void x_console_force_close(Console* console);

X_ConsoleCmd* x_console_get_cmd(Console* console, const char* cmdName);
bool x_console_cmd_exists(Console* console, const char* cmdName);
void x_console_register_cmd(Console* console, const char* name, X_ConsoleCmdHandler handler);

void x_console_register_var(Console* console, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, bool saveToConfig);
void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue);

void x_console_init(Console* console, struct X_EngineContext* engineContext, X_Font* font);
void x_console_cleanup(Console* console);

void x_console_clear(Console* console);
bool x_console_var_exists(Console* console, const char* name);
void x_console_print(Console* console, const char* str);
void x_console_printf(Console* console, const char* format, ...);
void x_console_render(Console* console);

void x_console_send_key(Console* console, X_Key key);

void x_console_execute_cmd(Console* console, const char* str);

