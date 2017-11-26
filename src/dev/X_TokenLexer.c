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

#include "X_TokenLexer.h"
#include "X_Console.h"

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
    return *lexer->inputStr == '\0' || *lexer->inputStr == ' ' || *lexer->inputStr == ';';
}

_Bool x_tokenlexer_lex_token(X_TokenLexer* lexer)
{
    if(*lexer->inputStr == ';')
    {
        *lexer->tokenBuf++ = *lexer->inputStr++;
        *lexer->tokenBuf++ = '\0';
        return 1;
    }
    
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

void x_tokenlexer_tokenize(X_TokenLexer* lexer)
{
    lexer->totalTokens = 0;
    
    while(x_tokenlexer_grab_next_token(lexer))
        ++lexer->totalTokens;
}
