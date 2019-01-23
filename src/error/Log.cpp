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

#include "engine/Config.hpp"
#include "system/File.hpp"
#include "system/FileSystem.hpp"
#include "error/Error.hpp"
#include "engine/Init.hpp"

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

static FILE* logFile;
static bool enableSafeLogging = 1;

void x_log_cleanup(void)
{
    fclose(logFile);
}

static void get_log_file_name(char* dest)
{
    FilePath path = FileSystem::getProgramPath();

    path.appendSegment("engine.log");

    printf("Log path: %s\n", path.c_str());

    strcpy(dest, path.c_str());
}

void x_log_init()
{
    char logFileName[X_FILENAME_MAX_LENGTH];
    get_log_file_name(logFileName);
    
    logFile = fopen(logFileName, "wb");

    if(logFile == nullptr)
    {
        x_system_error("Failed to open log file");
    }

    enableSafeLogging = 0;
}

static void flush_log_to_file(void)
{
    // x_file_close(&logFile);
    
    // char logFileName[X_FILENAME_MAX_LENGTH];
    // get_log_file_name(logFileName);
    
    // x_file_open_append(&logFile, logFileName);
}

void Log::info(const char* format, ...)
{
    const char* infoMessageColored = ANSI_COLOR_GREEN "[INFO]" ANSI_COLOR_RESET " ";
    const char* infoMessage = "[INFO] ";

    writeToLog(infoMessageColored, infoMessage);

    va_list list;
    va_start(list, format);
    writeToLog(format, list);

    writeToLog('\n');

    va_end(list);
}

void Log::error(const char* format, ...)
{
    const char* infoMessageColored = ANSI_COLOR_RED "[ERR ]" ANSI_COLOR_RESET " ";
    const char* infoMessage = "[ERR] ";

    writeToLog(infoMessageColored, infoMessage);

    va_list list;
    va_start(list, format);
    writeToLog(format, list);

    writeToLog('\n');

    va_end(list);
}

void Log::logSub(const char* format, ...)
{
    const char* subText = "\t- ";
    writeToLog(subText, subText);

    va_list list;
    va_start(list, format);
    writeToLog(format, list);

    writeToLog('\n');

    va_end(list);
}

void Log::writeToLog(const char* screenText, const char* fileText)
{
    fputs(screenText, stdout);
    fputs(fileText, logFile);
}

void Log::writeToLog(const char* format, va_list list)
{
    va_list fileList;
    va_copy(fileList, list);

    vprintf(format, list);
    vfprintf(logFile, format, fileList);

    va_end(fileList);
}

void Log::writeToLog(char c)
{
    fputc(c, stdout);
    fputc(c, logFile);
}

// TODO: take parameters into account
void Log::init(const char* logFile, bool enableLogging)
{
    x_log_init();
}

