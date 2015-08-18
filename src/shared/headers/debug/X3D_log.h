/* This file is part of X3D.
 *
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/// Types of information that can be sent to the log.
typedef enum {
  X3D_INFO = 1,
  X3D_WARNING = 2,
  X3D_ERROR = 4
} X3D_LogStatus;

/// Options for enabling logging
typedef enum {
  X3D_ENABLE_INFO = 1,
  X3D_ENABLE_WARNING = 2,
  X3D_ENABLE_ERROR = 4,
  
  X3D_ENABLE_LOG_STDOUT = 8,      /// Log will be echoed to stdout
  X3D_ENABLE_LOG_FILE = 16        /// Log will be saved to file "log"
} X3D_LogFlags;

/// Stores information about an X3D log
typedef struct X3D_Log {
  uint8 flags;    /// Log flags (see @ref X3D_LogFlags)
  FILE* file;     /// File to write log to
} X3D_Log;

//=============================================================================
// Static inline functions
//=============================================================================



//=============================================================================
// Functions
//=============================================================================

#ifdef __X3D_SHARED__

void x3d_log(X3D_LogStatus status, const char* format, ...);

#else

#define x3d_log _DLL_call(void, (X3D_LogStatus status, const char* format, ...), EXPORT_X3D_LOG)

#endif

