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
#include <string.h>

#include "engine/X_config.h"
#include "system/X_File.h"

#if X_ENABLE_COLOR_LOG

#define ANSI_COLOR_RED     "\x1b[1;31m"
#define ANSI_COLOR_GREEN   "\x1b[1;32m"
#define ANSI_COLOR_YELLOW  "\x1b[1;33m"
#define ANSI_COLOR_BLUE    "\x1b[1;34m"
#define ANSI_COLOR_MAGENTA "\x1b[1;35m"
#define ANSI_COLOR_CYAN    "\x1b[1;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#else

#define ANSI_COLOR_GREEN
#define ANSI_COLOR_RED
#define ANSI_COLOR_RESET

#endif

static X_File logFile;

void x_log_cleanup(void)
{
    x_file_close(&logFile);
}

void x_log_init(void)
{
    char logFileName[256];
    strcpy(logFileName, x_filesystem_get_program_path());
    strcat(logFileName, "/engine.log");
    
    if(!x_file_open_writing(&logFile, logFileName))
    {
        fprintf(stderr, "Failed to open log file\n");
        abort();
    }
}

void x_log(const char* format, ...)
{
    va_list list;
    const char* infoMessage = ANSI_COLOR_GREEN "[INFO]" ANSI_COLOR_RESET;
    
    printf("%s ", infoMessage);
    va_start(list, format);
    vprintf(format, list);
    printf("\n");
    
    if(!x_file_is_open(&logFile))
        return;
    
    va_start(list, format);
    
    fprintf(logFile.file, "[INFO] ");
    vfprintf(logFile.file, format, list);
    fputc('\n', logFile.file);
}

void x_log_error(const char* format, ...)
{
    va_list list;
    const char* errorMessage = ANSI_COLOR_RED "[ERR ]" ANSI_COLOR_RESET;
    
    printf("%s ", errorMessage);
    va_start(list, format);
    vprintf(format, list);
    printf("\n");

    if(!x_file_is_open(&logFile))
        return;
    
    
    va_start(list, format);
    
    fprintf(logFile.file, "[ERR ] ");
    vfprintf(logFile.file, format, list);
    fputc('\n', logFile.file);
}

