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

#include "dev/console/Console.hpp"
#include "engine/EngineContext.hpp"
#include "engine/Config.hpp"
#include "util/Util.hpp"
#include "dev/console/Console.hpp"
#include "error/Log.hpp"
#include "dev/console/AutoCompleter.hpp"
#include "TokenLexer.hpp"

void x_console_open(Console* console)
{
    console->renderer->show();
}

void x_console_close(Console* console)
{
    console->renderer->hide();
}

void x_console_force_close(Console* console)
{
    // console->openState = X_CONSOLE_STATE_CLOSED;
}

static void x_consolevar_init(X_ConsoleVar* consoleVar, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, bool saveToConfig)
{
    consoleVar->name = name;
    consoleVar->type = type;
    consoleVar->saveToConfig = saveToConfig;
    consoleVar->voidPtr = var;
    
    x_consolevar_set_value(consoleVar, initialValue);
}

static X_ConsoleVar* x_console_add_var(Console* console)
{
    ++console->totalConsoleVars;
    console->consoleVars = (X_ConsoleVar*)x_realloc(console->consoleVars, console->totalConsoleVars * sizeof(X_ConsoleVar));
    
    return console->consoleVars + console->totalConsoleVars - 1;    
}

void x_console_register_var(Console* console, void* var, const char* name, X_ConsoleVarType type, const char* initialValue, bool saveToConfig)
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

X_ConsoleCmd* x_console_get_cmd(Console* console, const char* cmdName)
{
    for(int i = 0; i < console->totalConsoleCmds; ++i)
    {
        if(strcmp(console->consoleCmds[i].name, cmdName) == 0)
            return console->consoleCmds + i;
    }
    
    return NULL;
}

bool x_console_cmd_exists(Console* console, const char* cmdName)
{
    return x_console_get_cmd(console, cmdName) != NULL;
}

static X_ConsoleCmd* x_console_add_cmd(Console* console)
{
    ++console->totalConsoleCmds;
    console->consoleCmds = (X_ConsoleCmd*)x_realloc(console->consoleCmds, console->totalConsoleCmds * sizeof(X_ConsoleCmd));
    
    return console->consoleCmds + console->totalConsoleCmds - 1;
}

void x_console_register_cmd(Console* console, const char* name, X_ConsoleCmdHandler handler)
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

static int x_console_bytes_in_line(const Console* console)
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
            
        case X_CONSOLEVAR_FP:
            *var->fpPtr = fp::fromFloat(atof(varValue));
            break;

        case X_CONSOLEVAR_BOOL:
            *var->boolPtr = (strcmp(varValue, "true") == 0 || atoi(varValue) != 0);
            break;

        case X_CONSOLEVAR_VEC3:
            float x, y, z;
            sscanf(varValue, "%f,%f,%f", &x, &y, &z);

            var->vec3Ptr->x = fp::fromFloat(x);
            var->vec3Ptr->y = fp::fromFloat(y);
            var->vec3Ptr->z = fp::fromFloat(z);

            break;
    }
}

void x_console_register_builtin_commands(Console* console);

Console::Console(EngineContext* engineContext, Font* font)
{
    cursor = Vec2(0, 0);

    Screen* screen = engineContext->screen;
    size.x = screen->getW() / font->getW();
    size.y = screen->getH() / font->getH() / 2;
    Console::font = font;
    Console::engineContext = engineContext;
    
    text = (char*)x_malloc(x_console_bytes_in_line(this) * size.y);
    x_console_clear(this);
    
    consoleCmds = NULL;
    totalConsoleCmds = 0;
    
    consoleVars = NULL;
    totalConsoleVars = 0;
    
    commandHistorySize = 0;
    commandHistoryPos = 0;
    
    for(int i = 0; i < X_CONSOLE_COMMAND_HISTORY_SIZE; ++i)
        x_string_init(commandHistory + i, "");
    
    x_console_register_builtin_commands(this);

    renderer = new ConsoleRenderer(*this, *engineContext->screen, *engineContext->mainFont);
}

void x_console_clear(Console* console)
{
    for(int i = 0; i < console->size.y; ++i)
        console->text[i * x_console_bytes_in_line(console)] = '\0';
    
    console->inputPos = 0;
    console->input[0] = '\0';
}

void x_console_cleanup(Console* console)
{
    x_free(console->text);
    
    for(int i = 0; i < X_CONSOLE_COMMAND_HISTORY_SIZE; ++i)
        x_string_cleanup(console->commandHistory + i);
    
    x_free(console->consoleCmds);
    x_free(console->consoleVars);
}

X_ConsoleVar* x_console_get_var(Console* console, const char* varName)
{
    for(int i = 0; i < console->totalConsoleVars; ++i)
    {
        if(strcmp(console->consoleVars[i].name, varName) == 0)
            return console->consoleVars + i;
    }
    
    return NULL;
}

bool x_console_var_exists(Console* console, const char* name)
{
    return x_console_get_var(console, name) != NULL;
}

// Prints a character to the current line (without wrapping) and advances the cursor
static void x_console_print_char(Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
    ++console->cursor.x;
}

// Prints a characeter to the currentline (without wrapping) without advancing the cursor
static void x_console_print_char_no_advance(Console* console, char c)
{
    console->text[console->cursor.y * x_console_bytes_in_line(console) + console->cursor.x] = c;
}

static char* x_console_start_of_line(Console* console, int line)
{
    return console->text + line * x_console_bytes_in_line(console);
}

static void x_console_scroll_one_line(Console* console)
{
    memmove(console->text, x_console_start_of_line(console, 1), x_console_bytes_in_line(console) * (console->size.y - 1));
    --console->cursor.y;
    x_console_print_char_no_advance(console, '\0');
}

// Moves to the next line, scrolling the console if necessary
static void x_console_newline(Console* console)
{
    x_console_print_char(console, '\0');
    
    console->cursor.x = 0;
    ++console->cursor.y;
    
    bool needsToScroll = console->cursor.y == console->size.y;
    if(needsToScroll)
        x_console_scroll_one_line(console);
}

// Determine whether counts as a character that's considered part of a word (for wrapping purposes)
static bool x_console_is_word_char(char c)
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

static bool x_console_word_should_wrap_to_next_line(Console* console, const char* wordStart, const char* wordEnd)
{
    int charsLeftOnCurrentLine = console->size.x - console->cursor.x - 1;
    int wordLength = wordEnd - wordStart;
    
    return charsLeftOnCurrentLine < wordLength;
}

static void x_console_print_handle_word_wrap(Console* console, const char** wordStart, const char* wordEnd)
{
    bool wrapToNextLine = x_console_word_should_wrap_to_next_line(console, *wordStart, wordEnd);
    if(wrapToNextLine)
        x_console_newline(console);
    
    if(**wordStart == ' ' && wrapToNextLine)
        ++(*wordStart);
}

static void x_console_print_word(Console* console, const char** wordStart)
{
    const char* wordEnd = find_end_of_word(*wordStart);
    x_console_print_handle_word_wrap(console, wordStart, wordEnd);
    
    while(*wordStart < wordEnd)
    {
        x_console_print_char(console, **wordStart);
        ++(*wordStart);
    }
}

void x_console_print(Console* console, const char* str)
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

void x_console_printf(Console* console, const char* format, ...)
{
    va_list list;
    
    va_start(list, format);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), format, list);
    
    x_console_print(console, buf);
    
    va_end(list);
}

static int x_console_h(const Console* console)
{
    return console->size.y * console->font->getH();
}

void x_console_set_var(Console* console, const char* varName, const char* varValue)
{
    X_ConsoleVar* var = x_console_get_var(console, varName);
    if(!var)
    {
        x_console_printf(console, "Variable %s not found\n", varName);
        return;
    }
    
    x_consolevar_set_value(var, varValue);
}

static void x_console_add_autcomplete_candidates(Console* console, X_AutoCompleter* ac)
{
    for(int i = 0; i < console->totalConsoleVars; ++i)
        x_autocompleter_add_match_candidate(ac, console->consoleVars[i].name);
    
    for(int i = 0; i < console->totalConsoleCmds; ++i)
        x_autocompleter_add_match_candidate(ac, console->consoleCmds[i].name);
}

static void x_console_add_trailing_space(Console* console)
{
    console->input[console->inputPos++] = ' ';
    console->input[console->inputPos] = '\0';
}

static bool x_console_autocomplete(Console* console, X_AutoCompleter* ac)
{
    if(!x_autocompleter_complete_partial_match(ac))
        return 0;
    
    console->inputPos = strlen(console->input);
    
    if(!x_autocompleter_has_exact_match(ac))
        return 0;
    
    x_console_add_trailing_space(console);
    return 1;
}

static void determine_column_widths(const char** items, int totalItems, int totalColumns, int* colWidths)
{
    for(int i = 0; i < totalColumns; ++i)
        colWidths[i] = 0;
    
    for(int col = 0; col < totalColumns; ++col)
    {
        for(int item = col; item < totalItems; item += totalColumns)
        {
            colWidths[col] = X_MAX(colWidths[col], (int)strlen(items[item]));
        }
    }
}

static bool row_exceeds_width_of_console(Console* console, int row, int totalColumns, int* colWidths, int totalItems, int colSpacing)
{
    int totalWidth = 0;
    for(int col = 0; col < totalColumns; ++col)
    {
        int item = row * totalColumns + col;
        if(item >= totalItems)
            break;
        
        totalWidth += colWidths[col];
        
        if(col != totalColumns - 1)
            totalWidth += colSpacing;
    }
    
    return totalWidth >= console->size.x;
}

static bool columns_widths_exceed_width_of_console(Console* console, int totalColumns, int* colWidths, int totalItems, int colSpacing)
{
    int totalRows = (totalItems + totalColumns - 1) / totalColumns;
    for(int row = 0; row < totalRows; ++row)
    {
        if(row_exceeds_width_of_console(console, row, totalColumns, colWidths, totalItems, colSpacing))
            return 1;
    }
    
    return 0;
}

static int determine_max_number_of_items_per_row(Console* console, const char** items, int totalItems, int maxItemsPerRow, int* colWidths, int itemSpacing)
{
    int matchesPerRow = maxItemsPerRow;
    
    do
    {
        determine_column_widths(items, totalItems, matchesPerRow, colWidths);
        
        if(!columns_widths_exceed_width_of_console(console, matchesPerRow, colWidths, totalItems, itemSpacing))
            break;
    } while(--matchesPerRow > 1);
    
    return matchesPerRow;
}

static void print_items_in_columns(Console* console, const char** items, int totalItems, int totalColumns, int* colWidths, int itemSpacing)
{    
    for(int match = 0; match < totalItems; ++match)
    {
        int row = match / totalColumns;
        int col = match % totalColumns;
        
        if(col == 0 && row != 0)
            x_console_print(console, "\n");
        
        x_console_print(console, items[match]);
        
        int spacing = (col != totalColumns - 1 ? itemSpacing : 0);
        
        for(int i = 0; i < colWidths[col] - (int)strlen(items[match]) + spacing; ++i)
            x_console_print(console, " ");
    }
    
    x_console_print(console, "\n");
}

static void print_items_in_columns_fit_to_console(Console* console, const char** items, int totalItems, int itemSpacing)
{
    const int MAX_MATCHES_PER_ROW = 10;
    int colWidths[MAX_MATCHES_PER_ROW];
    int itemsPerRow = determine_max_number_of_items_per_row(console, items, totalItems, MAX_MATCHES_PER_ROW, colWidths, itemSpacing);
    
    print_items_in_columns(console, items, totalItems, itemsPerRow, colWidths, itemSpacing);
}

static void x_console_print_autocomplete_matches(Console* console, X_AutoCompleter* ac)
{
    x_autocompleter_sort_matches(ac);
    x_console_printf(console, "] %s\n\n", console->input);
    
    const int MATCH_SPACING = 3;
    print_items_in_columns_fit_to_console(console, ac->matches, ac->totalMatches, MATCH_SPACING);
    x_console_print(console, "\n");
}

static void handle_backspace_key(Console* console)
{
    if(console->inputPos > 0) {
        --console->inputPos;
        console->input[console->inputPos] = '\0';
    }
}

static void add_input_to_command_history(Console* console)
{
    if(console->commandHistorySize == X_CONSOLE_COMMAND_HISTORY_SIZE - 1)
    {
        x_string_cleanup(console->commandHistory + 0);
        
        for(int i = 0; i < X_CONSOLE_COMMAND_HISTORY_SIZE - 1; ++i)
            console->commandHistory[i] = console->commandHistory[i + 1];
        
        --console->commandHistorySize;
        x_string_init(console->commandHistory + X_CONSOLE_COMMAND_HISTORY_SIZE - 1, "");
    }
    
    x_string_assign(console->commandHistory + console->commandHistorySize++, console->input);
}

static void handle_enter_key(Console* console)
{
    add_input_to_command_history(console);
    console->commandHistoryPos = console->commandHistorySize;
    
    x_console_printf(console, "] %s\n", console->input);
    x_console_execute_cmd(console, console->input);
    
    console->inputPos = 0;
    console->input[0] = '\0';
}

static char* find_start_of_current_cmd(char* str, int start, int* len)
{
    char* search = str + start;
    
    while(search > str)
    {
        if(*search == ';')
        {
            do
            {
                ++search;
            } while(*search == ' ');
            
            *len = start - (search - str);
            return search;
        }
        
        --search;
    }
    
    *len = start;
    return str;
}

static void handle_tab_key(Console* console, KeyCode lastKeyPressed)
{
    const int MAX_MATCHES = 100;
    const char* matches[MAX_MATCHES];
    
    int cmdLength;
    char* currentCmd = find_start_of_current_cmd(console->input, console->inputPos, &cmdLength);
    
    X_AutoCompleter ac;
    x_autocompleter_init(&ac, currentCmd, cmdLength, matches, MAX_MATCHES);
    x_console_add_autcomplete_candidates(console, &ac);
    
    if(x_console_autocomplete(console, &ac) || lastKeyPressed != KeyCode::tab)
    {
        return;
    }
        
    x_console_print_autocomplete_matches(console, &ac);
    console->lastKeyPressed = KeyCode::invalid;
}

static void handle_character_key(Console* console, KeyCode key)
{
    if(console->inputPos + 1 < X_CONSOLE_INPUT_BUF_SIZE)
    {
        console->input[console->inputPos++] = keyCodeToChar(key);
    }
    
    console->input[console->inputPos] = '\0';
}

static void handle_up_key(Console* console)
{
    if(console->commandHistoryPos == 0)
        return;
    
    bool onCurrentInput = console->commandHistoryPos == console->commandHistorySize;
    
    if(onCurrentInput)
    {
        x_string_assign(console->commandHistory + console->commandHistorySize, console->input);
    }
    
    strcpy(console->input, console->commandHistory[--console->commandHistoryPos].data);
    console->inputPos = strlen(console->input);
}

static void handle_down_key(Console* console)
{
    if(console->commandHistoryPos == console->commandHistorySize)
    {
        return;
    }
    
    strcpy(console->input, console->commandHistory[++console->commandHistoryPos].data);
    console->inputPos = strlen(console->input);
}

static bool is_character_key(KeyCode key)
{
    int k = (int)key;

    return k < 128 && isprint(k);
}

void x_console_send_key(Console* console, KeyCode key)
{
    KeyCode lastKey = console->lastKeyPressed;
    console->lastKeyPressed = key;

    switch(key)
    {
        case KeyCode::backspace:
            handle_backspace_key(console);
            return;

        case KeyCode::enter:
            handle_enter_key(console);
            return;

        case KeyCode::tab:
            handle_tab_key(console, lastKey);
            return;

        case KeyCode::up:
            handle_up_key(console);
            return;

        case KeyCode::down:
            handle_down_key(console);
            return;
    }
    
    if(is_character_key(key))
    {
        handle_character_key(console, key);
        return;
    }
}

static void print_variable_value(Console* console, X_ConsoleVar* var)
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
            
        case X_CONSOLEVAR_FP:
            sprintf(varValue, "%f", var->fpPtr->toFloat());
            break;
        case X_CONSOLEVAR_BOOL:
            sprintf(varValue, "%s", *var->boolPtr ? "true" : "false");
            break;
        case X_CONSOLEVAR_VEC3:
            sprintf(
                varValue,
                "%f,%f,%f",
                var->vec3Ptr->x.toFloat(),
                var->vec3Ptr->y.toFloat(),
                var->vec3Ptr->z.toFloat());

            break;
    }
    
    x_console_printf(console, "Variable %s is currently %s\n", var->name, varValue);
}

static bool token_begins_comment_line(const char* token)
{
    return token[0] == '/' && token[1] == '/';
}

static bool x_tokenlexer_is_valid_console_input(X_TokenLexer* lexer, char** tokens)
{
    return !lexer->errorOccured &&
        lexer->totalTokens > 0 &&
        !token_begins_comment_line(tokens[0]);
}

static bool x_console_input_try_execute_command(Console* console, char** tokens, int totalTokens)
{
    X_ConsoleCmd* cmd = x_console_get_cmd(console, tokens[0]);
    
    if(cmd == NULL)
        return 0;
    
    cmd->handler(console->engineContext, totalTokens, tokens);
    return 1;
}

static bool x_console_input_try_set_variable(Console* console, char** tokens, int totalTokens)
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

static int count_tokens_in_cmd(char** tokens, int totalTokens)
{
    for(int i = 0; i < totalTokens; ++i)
    {
        if(strcmp(tokens[i], ";") == 0)
            return i;
    }
    
    return totalTokens;
}

static bool try_run_command(Console* console, char** tokens, int totalTokens)
{
    if(x_console_input_try_execute_command(console, tokens, totalTokens))
        return 1;
    
    if(x_console_input_try_set_variable(console, tokens, totalTokens))
        return 1;
    
    x_console_printf(console, "Unknown command or var %s\n", tokens[0]);
    return 0;
}

void x_console_execute_cmd(Console* console, const char* str)
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
    
    char** tokenStart = tokens;
    int tokensLeft = lexer.totalTokens;
    
    do
    {
        int totalTokens = count_tokens_in_cmd(tokenStart, tokensLeft);
        if(!try_run_command(console, tokenStart, totalTokens))
            return;
        
        tokensLeft -= totalTokens + 1;
        tokenStart += totalTokens + 1;
    } while(tokensLeft > 0);
}

#include "ConsoleRenderer.hpp"

void x_console_render(Console* console)
{
    console->renderer->render();
}


