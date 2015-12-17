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

#include "X3D_platform.h"

void x3d_error_throw(int16 code, const char* format, ...);

int16 x3d_error_setup_error_frame(X3D_ErrorFrame* frame);

/**
 * Loads the error handling interface.
 * 
 * @returns Nothing.
 * @note    For internal use only.
 */
static inline void x3d_error_load_interface(void) {
	x3d->error.throw_error = x3d_error_throw;
}

