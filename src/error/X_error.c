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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __nspire__
#include <libndls.h>
#endif

#include "X_log.h"
#include "X_error.h"

void x_assert_function(_Bool condition, const char* file, int line, const char* messageFormat, ...)
{
    if(condition)
        return;
    
    x_log_error("Assertion failed!\nFile: %s\nLine: %d\n\nMessage: %s\n", file, line, messageFormat);
    x_system_error("Assertion Failed (check log for more details)");
}

void __attribute__((noreturn)) x_system_error(const char* format, ...)
{
    char error[4096];
    
    va_list list;
    va_start(list, format);
    vsnprintf(error, sizeof(error), format, list);
    
    x_log_error("%s", error);
    
#ifdef __nspire__
    show_msgbox("System Error", error);
#else
    fprintf(stderr, "Fatal system error: %s\n", error);
#endif
    
    va_end(list);
    exit(-1);
}

