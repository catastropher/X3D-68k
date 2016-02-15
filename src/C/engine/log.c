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

#include <stdio.h>
#include <stdarg.h>

#include "X3D_log.h"

#define ANSI_COLOR_RED     "\x1b[1;31m"
#define ANSI_COLOR_GREEN   "\x1b[1;32m"
#define ANSI_COLOR_YELLOW  "\x1b[1;33m"
#define ANSI_COLOR_BLUE    "\x1b[1;34m"
#define ANSI_COLOR_MAGENTA "\x1b[1;35m"
#define ANSI_COLOR_CYAN    "\x1b[1;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

///////////////////////////////////////////////////////////////////////////////
/// Prints a message to the log.
/// 
/// @param type     - type of message being displayed
/// @param format   - printf-like format string
///////////////////////////////////////////////////////////////////////////////
void x3d_log(X3D_LogType type, const char* format, ...) {
#if !defined(__68k__)
  va_list list;
  const char* color;
  const char* status[3] = {
    ANSI_COLOR_GREEN "[INFO]" ANSI_COLOR_RESET,
    ANSI_COLOR_YELLOW "[WARN]" ANSI_COLOR_RESET,
    ANSI_COLOR_RED "[ERR ]" ANSI_COLOR_RESET
  };
  
  printf("%s ", status[type]);
  va_start(list, format);
  vprintf(format, list);
  printf("\n");
#endif
}
