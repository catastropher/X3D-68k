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

#include <cstdarg>

struct LogConfig;

class Log
{
public:
    static void init(const char* logFile, bool enableLogging);
    static void cleanup();

    static void info(const char* format, ...) __attribute__ ((format (printf, 1, 2)));
    static void error(const char* format, ...) __attribute__ ((format (printf, 1, 2)));
    static void logSub(const char* format, ...) __attribute__ ((format (printf, 1, 2)));

private:
    static void writeToLog(const char* format, va_list list);
    static void writeToLog(const char* screenText, const char* fileText);
    static void writeToLog(char c);
};

void x_log_init(void);
void x_log_cleanup(void);

// Here for backwards compatibility
#define x_log(_args...) Log::info(_args)
#define x_log_error(_args...) Log::error(_args)

