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
#include "X_Console.h"
#include "error/X_log.h"

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
    consoleVar->name = name;
    consoleVar->type = type;
    consoleVar->saveToConfig = saveToConfig;
    consoleVar->voidPtr = var;
    
    x_consolevar_set_value(consoleVar, initialValue);
}

static X_ConsoleVar* x_console_add_var(X_Console* console)
{
    ++console->totalConsoleVars;
    console->consoleVars = x_realloc(console->consoleVars, console->totalConsoleVars * sizeof(X_ConsoleVar));
    
    return console->consoleVars + console->totalConsoleVars - 1;    
}

void x_console_register_var(X_Console* console, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, _Bool saveToConfig)
{    
    if(x_console_cmd_exists(console, name))
    {
        x_console_printf(console, "Can't register variable %s, name is already used for command\n", name);
        return;
    }
    
    if(x_console_var_exists(console, name))
    {
        x_console_printf(console, "Can't register variable %s, already defined\n", name);
        return;
    }
    
    X_ConsoleVar* consoleVar = x_console_add_var(console);
    x_consolevar_init(consoleVar, var, name, type, initialValue, saveToConfig);
}

X_ConsoleCmd* x_console_get_cmd(X_Console* console, const char* cmdName)
{
    for(int i = 0; i < console->totalConsoleCmds; ++i)
    {
        if(strcmp(console->consoleCmds[i].name, cmdName) == 0)
            return console->consoleCmds + i;
    }
    
    return NULL;
}

_Bool x_console_cmd_exists(X_Console* console, const char* cmdName)
{
    return x_console_get_cmd(console, cmdName) != NULL;
}

static X_ConsoleCmd* x_console_add_cmd(X_Console* console)
{
    ++console->totalConsoleCmds;
    console->consoleCmds = x_realloc(console->consoleCmds, console->totalConsoleCmds * sizeof(X_ConsoleCmd));
    
    return console->consoleCmds + console->totalConsoleCmds - 1;
}

void x_console_register_cmd(X_Console* console, const char* name, X_ConsoleCmdHandler handler)
{    
    if(x_console_var_exists(console, name))
    {
        x_console_printf(console, "Can't register command %s, name is already used for variable\n", name);
        return;
    }
    
    if(x_console_cmd_exists(console, name))
    {
        x_console_printf(console, "Can't register command %s, already defined\n", name);
        return;
    }
    
    X_ConsoleCmd* cmd = x_console_add_cmd(console);
    cmd->name = name;
    cmd->handler = handler;
}

static int x_console_bytes_in_line(const X_Console* console)
{
    return console->size.x + 1;
}

void x_consolevar_set_value(X_ConsoleVar* var, const char* varValue)
{
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
    console->cursor = x_vec2_make(0, 0);
    console->size.x = x_screen_w(&engineContext->screen) / font->charW;
    console->size.y = x_screen_h(&engineContext->screen) / font->charH / 2;
    console->font = font;
    console->engineContext = engineContext;
    console->renderYOffset = 0;
    
    console->lastCursorBlink = x_enginecontext_get_time(engineContext);
    console->showCursor = 1;
    
    console->text = x_malloc(x_console_bytes_in_line(console) * console->size.y);
    x_console_clear(console);
    
    console->consoleCmds = NULL;
    console->totalConsoleCmds = 0;
    
    console->consoleVars = NULL;
    console->totalConsoleVars = 0;
    
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
    for(int i = 0; i < console->totalConsoleVars; ++i)
    {
        if(strcmp(console->consoleVars[i].name, varName) == 0)
            return console->consoleVars + i;
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
static _Bool x_console_is_word_char(char c)
{
    return c > ' ';
}

static const char* find_end_of_word(const char* str)
{
    const char* endOfWord = str;
    
    if(!x_console_is_word_char(*str))
        return str + 1;
    
    do
    {
        ++endOfWord;
    } while(x_console_is_word_char(*endOfWord));
    
    return endOfWord;
}

static _Bool x_console_word_should_wrap_to_next_line(X_Console* console, const char* wordStart, const char* wordEnd)
{
    int charsLeftOnCurrentLine = console->size.x - console->cursor.x - 1;
    int wordLength = wordEnd - wordStart;
    
    return charsLeftOnCurrentLine < wordLength;
}

static void x_console_print_handle_word_wrap(X_Console* console, const char** wordStart, const char* wordEnd)
{
    _Bool wrapToNextLine = x_console_word_should_wrap_to_next_line(console, *wordStart, wordEnd);
    if(wrapToNextLine)
        x_console_newline(console);
    
    if(**wordStart == ' ' && wrapToNextLine)
        ++(*wordStart);
}

static void x_console_print_word(X_Console* console, const char** wordStart)
{
    const char* wordEnd = find_end_of_word(*wordStart);
    x_console_print_handle_word_wrap(console, wordStart, wordEnd);
    
    while(*wordStart < wordEnd)
    {
        x_console_print_char(console, **wordStart);
        ++(*wordStart);
    }
}

void x_console_print(X_Console* console, const char* str)
{
    x_log(str);
    
    while(*str)
    {
        if(*str == '\n')
        {
            x_console_newline(console);
            ++str;
            continue;
        }
        
        x_console_print_word(console, &str);
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

static void x_console_autocomplete_add_option(X_Console* console, const char* option, int* minMatchLength, const char** minMatchStr)
{
    int inputLength = console->inputPos;
    int matchLength = x_count_prefix_match_length(*minMatchStr, option);
    
    if(matchLength < inputLength)
        return;
    
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

static _Bool x_console_autocomplete(X_Console* console)
{
    int minMatchLength = 0x7FFFFFFF;
    const char* minMatchStr = console->input;
    
    for(int i = 0; i < console->totalConsoleVars; ++i)
        x_console_autocomplete_add_option(console, console->consoleVars[i].name, &minMatchLength, &minMatchStr);
    
    for(int i = 0; i < console->totalConsoleCmds; ++i)
        x_console_autocomplete_add_option(console, console->consoleCmds[i].name, &minMatchLength, &minMatchStr);
    
    _Bool foundStringToAutocompleteFrom = minMatchStr != console->input;
    if(foundStringToAutocompleteFrom)
    {
        x_strncpy(console->input, minMatchStr, minMatchLength);
        console->inputPos = strlen(console->input);
    }
    
    _Bool autoCompletePerfectMatch = (minMatchLength == strlen(minMatchStr));
    return autoCompletePerfectMatch;
}

#define MATCHES_PER_ROW 4
#define MATCH_SPACING 4

static int compare_matches(const void* a, const void* b)
{
    const char* strA = *((const char **)a);
    const char* strB = *((const char **)b);
    
    return strcmp(strA, strB);
}

static void x_console_print_autocomplete_matches(X_Console* console)
{
    const int MAX_AUTOCOMPLETE_MATCHES = 128;
    const char* autocompleteMatches[MAX_AUTOCOMPLETE_MATCHES];
    int totalAutocompleteMatches = 0;
    
    int inputLength = console->inputPos;
    
    for(int i = 0; i < console->totalConsoleVars; ++i)
    {
        if(x_count_prefix_match_length(console->input, console->consoleVars[i].name) == inputLength)
            autocompleteMatches[totalAutocompleteMatches++] = console->consoleVars[i].name;
    }
    
    for(int i = 0; i < console->totalConsoleCmds; ++i)
    {
        if(x_count_prefix_match_length(console->input, console->consoleCmds[i].name) == inputLength)
            autocompleteMatches[totalAutocompleteMatches++] = console->consoleCmds[i].name;
    }
    
    qsort(autocompleteMatches, totalAutocompleteMatches, sizeof(char**), compare_matches);
    
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

static void handle_backspace_key(X_Console* console)
{
    if(console->inputPos > 0) {
        --console->inputPos;
        console->input[console->inputPos] = '\0';
    }
}

static void handle_enter_key(X_Console* console)
{
    x_console_printf(console, "] %s\n", console->input);
    x_console_execute_cmd(console, console->input);
    
    console->inputPos = 0;
    console->input[0] = '\0';
}

static void handle_tab_key(X_Console* console, X_Key lastKeyPressed)
{
    if(x_console_autocomplete(console) || lastKeyPressed == '\t')
        return;
        
    x_console_print_autocomplete_matches(console);
    console->lastKeyPressed = 0;
}

static void handle_character_key(X_Console* console, X_Key key)
{
    if(console->inputPos + 1 < X_CONSOLE_INPUT_BUF_SIZE)
        console->input[console->inputPos++] = key;
    
    console->input[console->inputPos] = '\0';
}

static _Bool is_character_key(X_Key key)
{
    return key < 128 && isprint(key);
}

void x_console_send_key(X_Console* console, X_Key key)
{
    X_Key lastKey = console->lastKeyPressed;
    console->lastKeyPressed = key;
    
    if(key == '\b')
    {
        handle_backspace_key(console);
        return;
    }
    
    if(key == '\n')
    {
        handle_enter_key(console);
        return;
    }
    
    if(key == '\t')
    {
        handle_tab_key(console, lastKey);
        return;
    }
    
    if(is_character_key(key))
    {
        handle_character_key(console, key);
        return;
    }
}

typedef struct X_TokenLexer
{
    X_Console* console;
    const char* inputStr;
    char* tokenBuf;
    char* tokenBufEnd;
    char** tokens;
    char** tokensEnd;
    _Bool errorOccured;
    int totalTokens;
} X_TokenLexer;

void x_tokenlexer_init(X_TokenLexer* lexer, const char* inputStr, char* tokenBuf, int tokenBufSize, char** tokens, int maxTokens, X_Console* console)
{
    lexer->errorOccured = 0;
    lexer->inputStr = inputStr;
    lexer->tokenBuf = tokenBuf;
    lexer->tokenBufEnd = tokenBuf + tokenBufSize;
    lexer->tokens = tokens;
    lexer->tokensEnd = tokens + maxTokens;
    lexer->console = console;
}

static void x_tokenlexer_skip_whitespace(X_TokenLexer* lexer)
{
    while(*lexer->inputStr == ' ')
        ++lexer->inputStr;
}

static _Bool x_tokenlexer_at_end_of_token(const X_TokenLexer* lexer)
{
    return *lexer->inputStr == '\0' || *lexer->inputStr == ' ';
}

static _Bool x_tokenlexer_lex_token(X_TokenLexer* lexer)
{
    while(!x_tokenlexer_at_end_of_token(lexer))
    {
        if(lexer->tokenBuf == lexer->tokenBufEnd)
        {
            x_console_print(lexer->console, "Can't execute command (command too long)\n");
            lexer->errorOccured = 1;
            return 0;
        }
        
        *lexer->tokenBuf++ = *lexer->inputStr++;
    }
    
    *lexer->tokenBuf++ = '\0';
    
    return 1;
}

static _Bool x_tokenlexer_grab_next_token(X_TokenLexer* lexer)
{
    if(lexer->tokens == lexer->tokensEnd)
    {
        x_console_print(lexer->console, "Can't execute command (too many tokens)\n");
        lexer->errorOccured = 1;
        return 0;
    }
    
    x_tokenlexer_skip_whitespace(lexer);
    
    if(*lexer->inputStr == '\0')
        return 0;
    
    *lexer->tokens++ = lexer->tokenBuf;
    
    return x_tokenlexer_lex_token(lexer);
}

static void x_tokenlexer_tokenize(X_TokenLexer* lexer)
{
    lexer->totalTokens = 0;
    
    while(x_tokenlexer_grab_next_token(lexer))
        ++lexer->totalTokens;
}

static void print_variable_value(X_Console* console, X_ConsoleVar* var)
{
    char varValue[1024];
    
    switch(var->type)
    {
        case X_CONSOLEVAR_INT:
            sprintf(varValue, "%d", *var->intPtr);
            break;
            
        case X_CONSOLEVAR_FLOAT:
            sprintf(varValue, "%f", *var->floatPtr);
            break;
            
        case X_CONSOLEVAR_STRING:
            sprintf(varValue, "%s", var->stringPtr->data);
            break;
            
        case X_CONSOLEVAR_FP16X16:
            sprintf(varValue, "%f", x_fp16x16_to_float(*var->fp16x16Ptr));
            break;
        case X_CONSOLEVAR_BOOL:
            sprintf(varValue, "%s", *var->boolPtr ? "true" : "false");
            break;
    }
    
    x_console_printf(console, "Variable %s is currently %s\n", var->name, varValue);
}

static _Bool token_begins_comment_line(const char* token)
{
    return token[0] == '/' && token[1] == '/';
}

static _Bool x_tokenlexer_is_valid_console_input(X_TokenLexer* lexer, char** tokens)
{
    return !lexer->errorOccured &&
        lexer->totalTokens > 0 &&
        !token_begins_comment_line(tokens[0]);
}

static _Bool x_console_input_try_execute_command(X_Console* console, char** tokens, int totalTokens)
{
    X_ConsoleCmd* cmd = x_console_get_cmd(console, tokens[0]);
    
    if(cmd == NULL)
        return 0;
    
    cmd->handler(console->engineContext, totalTokens, tokens);
    return 1;
}

static _Bool x_console_input_try_set_variable(X_Console* console, char** tokens, int totalTokens)
{
    X_ConsoleVar* var = x_console_get_var(console, tokens[0]);
    if(!var)
        return 0;
    
    if(totalTokens > 2)
    {
        x_console_print(console, "Expected syntax <var> <value to set to>\n");
        return 1;
    }
    
    if(totalTokens == 1)
    {
        print_variable_value(console, var);
        return 1;
    }
    
    x_consolevar_set_value(var, tokens[1]);
    return 1;
}

void x_console_execute_cmd(X_Console* console, const char* str)
{
    const int MAX_TOKENS = 512;
    char* tokens[MAX_TOKENS];
    
    const int TOKEN_BUF_SIZE = 1024;
    char tokenBuf[TOKEN_BUF_SIZE];
    
    X_TokenLexer lexer;
    x_tokenlexer_init(&lexer, str, tokenBuf, TOKEN_BUF_SIZE, tokens, MAX_TOKENS, console);
    x_tokenlexer_tokenize(&lexer);
    
    if(!x_tokenlexer_is_valid_console_input(&lexer, tokens))
        return;
    
    if(x_console_input_try_execute_command(console, tokens, lexer.totalTokens))
        return;
    
    if(x_console_input_try_set_variable(console, tokens, lexer.totalTokens))
        return;
    
    x_console_printf(console, "Unknown command or var %s\n", tokens[0]);
}

