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

struct Console;

typedef struct X_TokenLexer
{
    struct Console* console;
    const char* inputStr;
    char* tokenBuf;
    char* tokenBufEnd;
    char** tokens;
    char** tokensEnd;
    bool errorOccured;
    int totalTokens;
} X_TokenLexer;

void x_tokenlexer_init(X_TokenLexer* lexer, const char* inputStr, char* tokenBuf, int tokenBufSize, char** tokens, int maxTokens, struct Console* console);
void x_tokenlexer_tokenize(X_TokenLexer* lexer);

