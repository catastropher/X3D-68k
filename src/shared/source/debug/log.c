// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_engine.h"
#include "debug/X3D_log.h"

#include <stdarg.h>
#include <stdio.h>

void x3d_log(X3D_LogStatus status, const char* format, ...) {
  X3D_Context* context = x3d_get_active_context();
  
  // Check if logging
  if((context->log.flags & status) == 0)
    return;
  
  const char* log_status[] = {
    "INFO",
    "WARN",
    "",
    "ERR "
  };
  
  va_list list;
  
  if((context->log.flags & X3D_ENABLE_LOG_STDOUT) != 0) {
    va_start(list, format);
    printf("[%s] ", log_status[status - 1]);
    vprintf(format, &list);
  }
  
  if((context->log.flags & X3D_ENABLE_LOG_FILE) != 0) {
    va_start(list, format);
    fprintf(context->log.file, "%s ", log_status[status - 1]);
    vfprintf(context->log.file, format, &list);
  }
  
  printf("Flags: %d\n", context->log.flags);
}

