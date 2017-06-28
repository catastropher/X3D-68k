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
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "X_Console.h"

static void x_consolevar_init(X_ConsoleVar* consoleVar, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig)
{
    x_string_init(&consoleVar->assignedValueString, "");
    
    consoleVar->name = name;
    consoleVar->type = type;
    consoleVar->saveToConfig = saveToConfig;
    consoleVar->next = NULL;
    consoleVar->voidPtr = var;
    
    x_consolevar_set_value(consoleVar, initialValue);
}

void x_console_register_var(X_Console* console, X_ConsoleVar* consoleVar, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig)
{
    x_consolevar_init(consoleVar, var, name, type, initialValue, saveToConfig);
    
    if(x_console_var_exists(console, consoleVar->name))
    {
        x_console_printf(console, "Can't register variable %s, already defined\n", consoleVar->name);
        return;
    }
    
    consoleVar->next = console->consoleVarsHead;
    console->consoleVarsHead = consoleVar;
}

static int x_console_bytes_in_line(const X_Console* console)
{
    return console->size.x + 1;
}

void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue)
{
    x_string_assign(&var->assignedValueString, varValue);
    /// @TODO maybe add type checking?
    
    switch(var->type)
    {
        case X_CONSOLEVAR_INT:
            *var->intPtr = atoi(varValue);
            break;
            
        case X_CONSOLEVAR_FLOAT:
            *var->floatPtr = atoi(varValue);
            break;
            
        case X_CONSOLEVAR_STRING:
            x_string_assign(var->stringPtr, varValue);
            break;
            
        case X_CONSOLEVAR_FP16X16:
            *var->fp16x16Ptr = x_fp16x16_from_float(atof(varValue));
            break;
        case X_CONSOLEVAR_BOOL:
            *var->boolPtr = (strcmp(varValue, "true") == 0 || atoi(varValue) != 0);
            break;
    }
}

void x_console_init(X_Console* console, X_Screen* screen, X_Font* font)
{
    console->consoleVarsHead = NULL;
    console->isOpen = 0;
    console->cursor = x_vec2_make(0, 0);
    console->size.x = x_screen_w(screen) / font->charW;
    console->size.y = x_screen_h(screen) / font->charH / 2;
    console->font = font;
    console->screen = screen;
    
    console->text = x_malloc(x_console_bytes_in_line(console) * console->size.y);
    x_console_clear(console);
    
    // FIXME: the palette hasn't been initialized by the time this is called
    // console->backgroundColor = x_palette_get_closest_color_from_rgb(screen->palette, 0, 0, 0);
}

void x_console_clear(X_Console* console)
{
    for(int i = 0; i < console->size.y; ++i)
        console->text[i * x_console_bytes_in_line(console)] = '\0';
    
    console->inputPos = 0;
    console->input[0] = '\0';
}

void x_console_cleanup(X_Console* console)
{
    x_free(console->text);
}

X_ConsoleVar* x_console_get_var(X_Console* console, const char* varName)
{
    X_ConsoleVar* var = console->consoleVarsHead;
    while(var)
    {
        if(strcmp(var->name, varName) == 0)
            return var;
        
        var = var->next;
    }
    
    return NULL;
}

_Bool x_console_var_exists(X_Console* console, const char* name)
{
    return x_console_get_var(console, name) != NULL;
}

static void x_console_print_char(X_Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
    ++console->cursor.x;
}

static void x_console_print_char_no_advance(X_Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
}

static void x_console_newline(X_Console* console)
{
    x_console_print_char(console, '\0');
    
    console->cursor.x = 0;
    ++console->cursor.y;
    
    _Bool needsToScroll = console->cursor.y == console->size.y;
    if(needsToScroll)
    {
        memmove(console->text, console->text + x_console_bytes_in_line(console), x_console_bytes_in_line(console) * (console->size.y - 1));
        --console->cursor.y;
        x_console_print_char_no_advance(console, '\0');
    }
}

static _Bool is_word_char(char c)
{
    return c > ' ';
}

static const char* find_end_of_word(const char* str)
{
    const char* endOfWord = str;
    
    if(!is_word_char(*str))
        return str + 1;
    
    do
    {
        ++endOfWord;
    } while(is_word_char(*endOfWord));
    
    return endOfWord;
}

void x_console_print(X_Console* console, const char* str)
{
    while(*str)
    {
        if(*str == '\n')
        {
            x_console_newline(console);
            ++str;
            continue;
        }
        
        const char* endOfWord = find_end_of_word(str);
        int charsLeftOnCurrentLine = console->size.x - console->cursor.x - 1;
        int wordLength = endOfWord - str;
        
        _Bool wrapToNextLine = charsLeftOnCurrentLine < wordLength;
        if(wrapToNextLine)
            x_console_newline(console);
        
        if(*str == ' ' && wrapToNextLine)
            ++str;
        
        while(str < endOfWord)
            x_console_print_char(console, *str++);
    }
    
    x_console_print_char_no_advance(console, '\0');
}

void x_console_printf(X_Console* console, const char* format, ...)
{
    va_list list;
    
    va_start(list, format);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, list);
    
    x_console_print(console, buf);
}

static void x_console_render_input(X_Console* console)
{
    unsigned char cursorChar = 11;
    console->input[console->inputPos] = cursorChar;
    console->input[console->inputPos + 1] = '\0';
    
    char* input = console->input;
    
    if(console->inputPos + 1 >= console->size.x)
    {
        int charsToScrollHorizontallyBy = console->inputPos + 1 - console->size.x;
        input += charsToScrollHorizontallyBy;
    }
    
    x_canvas_draw_str(&console->screen->canvas, input, console->font, x_vec2_make(0, console->size.y * console->font->charH));
    
    console->input[console->inputPos] = '\0';
}

void x_console_render_background(X_Console* console)
{
    console->backgroundColor = x_palette_get_closest_color_from_rgb(console->screen->palette, 0, 0, 0);
    x_canvas_fill_rect(&console->screen->canvas, x_vec2_make(0, 0), x_vec2_make(x_screen_w(console->screen) - 1, (console->size.y + 1) * console->font->charH - 1), console->backgroundColor);
}

void x_console_render(X_Console* console)
{
    x_console_render_background(console);
    
    for(int i = 0; i < console->size.y; ++i)
        x_canvas_draw_str(&console->screen->canvas, console->text + i * x_console_bytes_in_line(console), console->font, x_vec2_make(0, i * console->font->charH));
    
    x_console_render_input(console);
}

void x_console_set_var(X_Console* console, const char* varName, const char* varValue)
{
    X_ConsoleVar* var = x_console_get_var(console, varName);
    if(!var)
    {
        x_console_printf(console, "Variable %s not found\n", varName);
        return;
    }
    
    x_consolevar_set_value(var, varValue);
}

void x_console_send_key(X_Console* console, X_Key key)
{
    if(key == '\b')
    {
        if(console->inputPos > 0) {
            --console->inputPos;
            console->input[console->inputPos] = '\0';
        }
        
        return;
    }
    
    if(key == '\n')
    {
        x_console_printf(console, "%s\n", console->input);
        x_console_execute_cmd(console, console->input);
        
        console->inputPos = 0;
        console->input[0] = '\0';
        
        return;
    }
    
    if(key < 128 && isprint(key))
    {
        if(console->inputPos + 1 < X_CONSOLE_INPUT_BUF_SIZE)
            console->input[console->inputPos++] = key;
        
        console->input[console->inputPos] = '\0';        
        return;
    }
}

typedef struct TokenContext
{
    X_Console* console;
    const char* str;
    char* tokenBuf;
    char* tokenBufEnd;
    char** tokens;
    char** tokensEnd;
    _Bool errorOccured;
    int totalTokens;
} TokenContext;

static void skip_whitespace(TokenContext* c)
{
    while(*c->str == ' ')
        ++c->str;
}

static _Bool grab_next_token(TokenContext* c)
{
    if(c->tokens == c->tokensEnd)
    {
        x_console_print(c->console, "Can't execute command (too many tokens)\n");
        c->errorOccured = 1;
        return 0;
    }
    
    skip_whitespace(c);
    
    if(*c->str == '\0')
        return 0;
    
    *c->tokens++ = c->tokenBuf;
    
    while(*c->str != '\0' && *c->str != ' ')
    {
        if(c->tokenBuf == c->tokenBufEnd)
        {
            x_console_print(c->console, "Can't execute command (command too long)\n");
            c->errorOccured = 1;
            return 0;
        }
        
        *c->tokenBuf++ = *c->str++;
    }
    
    *c->tokenBuf++ = '\0';
    
    return 1;
}

static void tokenize(TokenContext* c)
{
    c->totalTokens = 0;
    
    while(grab_next_token(c))
        ++c->totalTokens;
}

static void print_value_of_variable(X_Console* console, const char* varName)
{
    X_ConsoleVar* var = x_console_get_var(console, varName);
    if(!var)
    {
        x_console_printf(console, "Unknown variable %s\n", varName);
        return;
    }
    
    x_console_printf(console, "Variable %s is currently %s\n", varName, var->assignedValueString);
}

static void set_variable_value(X_Console* console, const char* varName, const char* varValue)
{
    X_ConsoleVar* var = x_console_get_var(console, varName);
    if(!var)
    {
        x_console_printf(console, "Unknown variable %s\n", varName);
        return;
    }
    
    x_consolevar_set_value(var, varValue);
}

#define MAX_TOKENS 512
#define TOKEN_BUF_SIZE 1024


void x_console_execute_cmd(X_Console* console, const char* str)
{
    char tokenBuf[TOKEN_BUF_SIZE];
    char* tokens[MAX_TOKENS];
    
    TokenContext context;
    context.errorOccured = 0;
    context.str = str;
    context.tokenBuf = tokenBuf;
    context.tokenBufEnd = tokenBuf + TOKEN_BUF_SIZE;
    context.tokens = tokens;
    context.tokensEnd = tokens + MAX_TOKENS;
    
    tokenize(&context);
    
    if(context.errorOccured)
        return;
    
    if(context.totalTokens == 0)
        return;
    else if(context.totalTokens == 1)
        print_value_of_variable(console, tokens[0]);
    else if(context.totalTokens == 2)
        set_variable_value(console, tokens[0], tokens[1]);
    
}

