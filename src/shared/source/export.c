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

#include "X3D_config.h"
#include "debug/X3D_log.h"
#include "debug/X3D_error.h"
#include "resource/X3D_memory.h"
#include "X3D_trig.h"

#include "init/X3D_init.h"


#ifdef __TIGCC__

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

DLL_INTERFACE

DLL_ID X3D_ID
DLL_VERSION 1,0
DLL_EXPORTS x3d_init_core, x3d_log, x3d_cleanup_core, &x3d_loaded, x3d_error, x3d_malloc, x3d_free, x3d_realloc, x3d_sinfp

DLL_IMPLEMENTATION


#endif

