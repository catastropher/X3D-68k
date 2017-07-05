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
#include "engine/X_EngineContext.h"
#include "engine/X_config.h"
#include "util/X_util.h"

void x_console_open(X_Console* console)
{
    console->consoleToggleTime = x_enginecontext_get_time(console->engineContext);
    console->openState = X_CONSOLE_STATE_OPENING;
}

void x_console_close(X_Console* console)
{
    console->consoleToggleTime = x_enginecontext_get_time(console->engineContext);
    console->openState = X_CONSOLE_STATE_CLOSING;
}

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
    
    if(x_console_cmd_exists(console, consoleVar->name))
    {
        x_console_printf(console, "Can't register variable %s, name is already used for command\n", consoleVar->name);
        return;
    }
    
    if(x_console_var_exists(console, consoleVar->name))
    {
        x_console_printf(console, "Can't register variable %s, already defined\n", consoleVar->name);
        return;
    }
    
    consoleVar->next = console->consoleVarsHead;
    console->consoleVarsHead = consoleVar;
}

X_ConsoleCmd* x_console_get_cmd(X_Console* console, const char* cmdName)
{
    X_ConsoleCmd* cmd = console->consoleCmdHead;
    while(cmd)
    {
        if(strcmp(cmd->name, cmdName) == 0)
            return cmd;
        
        cmd = cmd->next;
    }
    
    return NULL;
}

_Bool x_console_cmd_exists(X_Console* console, const char* cmdName)
{
    return x_console_get_cmd(console, cmdName) != NULL;
}

void x_console_register_cmd(X_Console* console, X_ConsoleCmd* cmd)
{
    if(x_console_var_exists(console, cmd->name))
    {
        x_console_printf(console, "Can't register command %s, name is already used for variable\n", cmd->name);
        return;
    }
    
    if(x_console_cmd_exists(console, cmd->name))
    {
        x_console_printf(console, "Can't register command %s, already defined\n", cmd->name);
        return;
    }
    
    cmd->next = console->consoleCmdHead;
    console->consoleCmdHead = cmd;
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

void x_console_register_builtin_commands(X_Console* console);

void x_console_init(X_Console* console, X_EngineContext* engineContext, X_Font* font)
{
    console->openState = X_CONSOLE_STATE_CLOSED;
    console->consoleVarsHead = NULL;
    console->consoleCmdHead = NULL;
    console->cursor = x_vec2_make(0, 0);
    console->size.x = x_screen_w(&engineContext->screen) / font->charW;
    console->size.y = x_screen_h(&engineContext->screen) / font->charH / 2;
    console->font = font;
    console->engineContext = engineContext;
    
    console->lastCursorBlink = x_enginecontext_get_time(engineContext);
    console->showCursor = 1;
    
    console->text = x_malloc(x_console_bytes_in_line(console) * console->size.y);
    x_console_clear(console);
    
    x_console_register_builtin_commands(console);
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

// Prints a character to the current line (without wrapping) and advances the cursor
static void x_console_print_char(X_Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
    ++console->cursor.x;
}

// Prints a characeter to the currentline (without wrapping) without advancing the cursor
static void x_console_print_char_no_advance(X_Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
}

static char* x_console_start_of_line(X_Console* console, int line)
{
    return console->text + line * x_console_bytes_in_line(console);
}

static void x_console_scroll_one_line(X_Console* console)
{
    memmove(console->text, x_console_start_of_line(console, 1), x_console_bytes_in_line(console) * (console->size.y - 1));
    --console->cursor.y;
    x_console_print_char_no_advance(console, '\0');
}

// Moves to the next line, scrolling the console if necessary
static void x_console_newline(X_Console* console)
{
    x_console_print_char(console, '\0');
    
    console->cursor.x = 0;
    ++console->cursor.y;
    
    _Bool needsToScroll = console->cursor.y == console->size.y;
    if(needsToScroll)
        x_console_scroll_one_line(console);
}

// Determine whether counts as a character that's considered part of a word (for wrapping purposes)
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
    char buf[2048];
    vsnprintf(buf, sizeof(buf), format, list);
    
    x_console_print(console, buf);
}

static void x_console_handle_cursor_blinking(X_Console* console)
{
    X_Time currentTime = x_enginecontext_get_time(console->engineContext);
    
    if(currentTime - console->lastCursorBlink >= X_CONSOLE_CURSOR_BLINK_TIME)
    {
        console->showCursor = !console->showCursor;
        console->lastCursorBlink = currentTime;
    }
}

static int x_console_line_y(X_Console* console, int lineNumber)
{
    return lineNumber * console->font->charH + console->renderYOffset;
}

static void x_console_add_cursor_to_input_buf(X_Console* console)
{
    const unsigned char CURSOR_CHAR = 11;
    console->input[console->inputPos] = CURSOR_CHAR;
    console->input[console->inputPos + 1] = '\0';
}

static char* x_console_get_start_of_scrolled_input(X_Console* console)
{
    char* input = console->input;
    int inputLenghtIncludingCursor = console->inputPos + 3; 
    
    if(inputLenghtIncludingCursor >= console->size.x)
    {
        int charsToScrollHorizontallyBy = inputLenghtIncludingCursor - console->size.x;
        input += charsToScrollHorizontallyBy;
    }
    
    return input;
}

static void x_console_remove_cursor_from_input_buf(X_Console* console)
{
    console->input[console->inputPos] = '\0';
}

static int x_console_get_next_empty_line(X_Console* console)
{
    return console->cursor.y + (console->cursor.x == 0 ? 0 : 1);
}

static int x_console_h(const X_Console* console)
{
    return console->size.y * console->font->charH;
}

static X_Screen* x_console_get_screen(X_Console* console)
{
    return &console->engineContext->screen;
}

static X_Canvas* x_console_get_canvas(X_Console* console)
{
    return &x_console_get_screen(console)->canvas;
}

static const X_Palette* x_console_get_palette(X_Console* console)
{
    return x_console_get_screen(console)->palette;
}

static void x_console_render_input(X_Console* console)
{
    x_console_handle_cursor_blinking(console);
    
    if(console->showCursor)
        x_console_add_cursor_to_input_buf(console);
        
    char* scrolledInput = x_console_get_start_of_scrolled_input(console);
    int inputLineY = x_console_line_y(console, x_console_get_next_empty_line(console));
    const int CHARS_IN_CURSOR = 2;
    
    X_Canvas* canvas = x_console_get_canvas(console);
    x_canvas_draw_char(canvas, ']', console->font, x_vec2_make(0, inputLineY));
    x_canvas_draw_str(canvas, scrolledInput, console->font, x_vec2_make(console->font->charW * CHARS_IN_CURSOR, inputLineY));
    
    x_console_remove_cursor_from_input_buf(console);
}

void x_console_render_background(X_Console* console)
{
    X_Screen* screen = x_console_get_screen(console);
    X_Canvas* canvas = x_console_get_canvas(console);
    const X_Palette* palette = x_console_get_palette(console);
    X_Color backgroundColor = palette->black;
    X_Color lineColor = palette->darkRed;
    
    X_Vec2 topLeft = x_vec2_make(0, x_console_line_y(console, 0));
    X_Vec2 bottomRight = x_vec2_make(x_screen_w(screen) - 1, x_console_line_y(console, console->size.y));
    
    x_canvas_fill_rect(canvas, topLeft, bottomRight, backgroundColor);
    
    X_Vec2 bottomLeft = x_vec2_make(0, bottomRight.y);
    x_canvas_draw_line(canvas, bottomLeft, bottomRight, lineColor);
}

static void x_console_handle_opening_animation(X_Console* console)
{
    int timePassed = x_enginecontext_get_time(console->engineContext) - console->consoleToggleTime;
    int consoleHeight = x_console_h(console);
    
    console->renderYOffset = -consoleHeight * (X_CONSOLE_TOGGLE_TIME - timePassed) / X_CONSOLE_TOGGLE_TIME;
    
    if(console->renderYOffset >= 0)
    {
        console->renderYOffset = 0;
        console->openState = X_CONSOLE_STATE_OPEN;
    }
}

static void x_console_handle_closing_animation(X_Console* console)
{
    int timePassed = x_enginecontext_get_time(console->engineContext) - console->consoleToggleTime;
    int consoleHeight = x_console_h(console);
    
    console->renderYOffset = -consoleHeight * timePassed / X_CONSOLE_TOGGLE_TIME;
    
    if(console->renderYOffset <= -consoleHeight)
        console->openState = X_CONSOLE_STATE_CLOSED;
}

static void x_console_handle_open_state_animation(X_Console* console)
{
    if(console->openState == X_CONSOLE_STATE_OPENING)
        x_console_handle_opening_animation(console);
    else if(console->openState == X_CONSOLE_STATE_CLOSING)
        x_console_handle_closing_animation(console);
}

static void x_console_render_text(X_Console* console)
{
    X_Canvas* canvas = x_console_get_canvas(console);
    for(int i = 0; i < console->size.y; ++i)
    {
        const char* startOfLine = console->text + i * x_console_bytes_in_line(console);
        x_canvas_draw_str(canvas, startOfLine, console->font, x_vec2_make(0, x_console_line_y(console, i)));
    }
}

void x_console_render(X_Console* console)
{
    x_console_handle_open_state_animation(console);
    
    if(!x_console_is_open(console))
        return;
    
    x_console_render_background(console);
    x_console_render_text(console);
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

static void x_console_autocomplete_add_option(X_Console* console, const char* option, int inputLength, int* minMatchLength, const char** minMatchStr)
{
    int matchLength = x_count_prefix_match_length(*minMatchStr, option);
        
    if(matchLength >= inputLength)
    {
        if(*minMatchStr == console->input)
        {
            *minMatchStr = option;
            *minMatchLength = strlen(option);
        }
        else if(matchLength < *minMatchLength) {
            *minMatchLength = matchLength;
            *minMatchStr = option;
        }
    }
}

static _Bool x_console_autocomplete(X_Console* console)
{
    int inputLength = console->inputPos;
    int minMatchLength = 0x7FFFFFFF;
    const char* minMatchStr = console->input;
    
    for(X_ConsoleVar* var = console->consoleVarsHead; var != NULL; var = var->next)
        x_console_autocomplete_add_option(console, var->name, inputLength, &minMatchLength, &minMatchStr);
    
    for(X_ConsoleCmd* cmd = console->consoleCmdHead; cmd != NULL; cmd = cmd->next)
        x_console_autocomplete_add_option(console, cmd->name, inputLength, &minMatchLength, &minMatchStr);
    
    if(minMatchStr != console->input)
    {
        x_strncpy(console->input, minMatchStr, minMatchLength);
        console->inputPos = strlen(console->input);
    }
    
    return minMatchLength == strlen(minMatchStr);
}

#define MATCHES_PER_ROW 4
#define MATCH_SPACING 4

static void x_console_print_autocomplete_matches(X_Console* console)
{
    const int MAX_AUTOCOMPLETE_MATCHES = 128;
    const char* autocompleteMatches[MAX_AUTOCOMPLETE_MATCHES];
    int totalAutocompleteMatches = 0;
    
    int inputLength = console->inputPos;
    
    for(X_ConsoleVar* var = console->consoleVarsHead; var != NULL; var = var->next)
    {
        if(x_count_prefix_match_length(console->input, var->name) == inputLength)
            autocompleteMatches[totalAutocompleteMatches++] = var->name;
    }
    
    for(X_ConsoleCmd* cmd = console->consoleCmdHead; cmd != NULL; cmd = cmd->next)
    {
        if(x_count_prefix_match_length(console->input, cmd->name) == inputLength)
            autocompleteMatches[totalAutocompleteMatches++] = cmd->name;
    }
    
    int colWidth[MATCHES_PER_ROW] = { 0 };
    
    for(int col = 0; col < MATCHES_PER_ROW; ++col)
    {
        for(int match = col; match < totalAutocompleteMatches; match += MATCHES_PER_ROW)
        {
            colWidth[col] = X_MAX(colWidth[col], strlen(autocompleteMatches[match]));
        }
    }
    
    x_console_printf(console, "] %s\n\n", console->input);
    
    for(int match = 0; match < totalAutocompleteMatches; ++match)
    {
        int row = match / MATCHES_PER_ROW;
        int col = match % MATCHES_PER_ROW;
        
        if(col == 0 && row != 0)
            x_console_print(console, "\n");
        
        x_console_print(console, autocompleteMatches[match]);
        
        for(int i = 0; i < colWidth[col] - strlen(autocompleteMatches[match]) + MATCH_SPACING; ++i)
            x_console_print(console, " ");
    }
    
    x_console_print(console, "\n\n");
}

void x_console_send_key(X_Console* console, X_Key key)
{
    X_Key lastKey = console->lastKeyPressed;
    console->lastKeyPressed = key;
    
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
        x_console_printf(console, "] %s\n", console->input);
        x_console_execute_cmd(console, console->input);
        
        console->inputPos = 0;
        console->input[0] = '\0';
        
        return;
    }
    
    if(key == '\t')
    {
        if(!x_console_autocomplete(console) && lastKey == '\t')
        {
            x_console_print_autocomplete_matches(console);
            console->lastKeyPressed = 0;
        }
        
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

static void print_variable_value(X_Console* console, const char* varName)
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
    
    X_ConsoleCmd* cmd = x_console_get_cmd(console, tokens[0]);
    
    if(cmd != NULL)
    {
        cmd->handler(console->engineContext, context.totalTokens, tokens);
        return;
    }
    
    if(context.totalTokens == 1)
        print_variable_value(console, tokens[0]);
    else if(context.totalTokens == 2)
        set_variable_value(console, tokens[0], tokens[1]);
    else
        x_console_print(console, "Bad command\n");          // Need better message
}

