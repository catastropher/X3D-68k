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

#include "X3D_common.h"

//#define X3D_NO_LOG

///////////////////////////////////////////////////////////////////////////////
/// The types of log messages that can be displayed
///////////////////////////////////////////////////////////////////////////////
typedef enum {
  X3D_INFO,
  X3D_WARN,
  X3D_ERROR
} X3D_LogType;

#ifndef X3D_NO_LOG

void x3d_log(X3D_LogType type, const char* format, ...) __attribute__ ((format(printf, 2, 3)));

#else

#define x3d_log(...)

#endif


