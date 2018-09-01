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

#include <cstdarg>
#include <cstdio>

#include "X_Log.hpp"
#include "error/X_Exception.hpp"

namespace X3D
{
    void Log::info(const char* format, ...)
    {
        va_list list;
        va_start(list, format);

        printf("[INFO] ");
        vprintf(format, list);
        printf("\n");

        va_end(list);
    }

    void Log::error(const char* format, ...)
    {
        va_list list;
        va_start(list, format);

        printf("[ERR ] ");
        vprintf(format, list);
        printf("\n");

        va_end(list);
    }

    void Log::error(const Exception& e, const char* format, ...)
    {
        va_list list;
        va_start(list, format);

        printf("[ERR ] ");
        vprintf(format, list);
        printf("\n");

        printf("\t- Exception: %s\n", e.getMessage());

        char details[1024];
        e.getDetails(details);

        printf("\t- Details: %s\n", details);

        va_end(list);
    }
};


