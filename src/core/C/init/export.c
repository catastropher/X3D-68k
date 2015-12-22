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

#include "X3D_core.h"

#ifdef __TIGCC__

#include <tigcclib.h>

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

void x3d_load_interface(void);

DLL_INTERFACE

void x3dcore_export_function() {
  
}

DLL_ID X3D_ID
DLL_VERSION 1,0
DLL_EXPORTS x3dcore_export_function

DLL_IMPLEMENTATION


#endif

