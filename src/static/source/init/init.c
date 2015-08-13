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
#include "X3D_fix.h"
#include "X3D_engine.h"
#include "init/X3D_init.h"

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

/**
 * Attempts to initialize X3D by loading the DLL and calling x3d_init_core(). 
 * 
*/
uint16 x3d_init(X3D_Context* context, X3D_Config* config) {
  uint16 dll_status;
  
#ifdef __TIGCC__
  if((dll_status = LoadDLL("x3d", X3D_ID, 1, 0)) != DLL_OK) {
    return dll_status;
  }
  x3d_init_core(context, config);
#endif
  
  return 0;
}

