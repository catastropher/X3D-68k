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

/// Asserts that a condition is true. If not, it throws an error.
/// @todo This should be wrapped in an ifdef if debugging is not enabled
#if 1
#define x3d_assert(_cond) if(!(_cond)){ x3d_platform_assert_failed(#_cond, __FILE__, __LINE__); }
#else
#define x3d_assert(...) ;
#endif

/// Asserts that a value is within a range
#define x3d_assert_range(_value, _begin, _end) x3d_assert(_value >= _begin && _value < _end)

///////////////////////////////////////////////////////////////////////////////
/// Throws an error because an assert failed (see @ref x3d_assert).
///
/// @param condition  - the condition that failed
/// @param file       - file where the assert failed
/// @param line       - line in the file where the assert failed
///
/// @note This function does not return.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL
X3D_PLATFORM
void x3d_platform_assert_failed(const char* condition, const char* file, int16 line);

