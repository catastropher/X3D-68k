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

#if 0
 
#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_engine.h"
#include "init/X3D_init.h"
#include "debug/X3D_static_error.h"

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')


CALLBACK void x3d_cleanup(void) {
  if(x3d_loaded) {
    x3d_cleanup_core();
    UnloadDLL();
  }
}

/**
 * Attempts to initialize X3D by loading the DLL and calling x3d_init_core(). 
 * 
 */
uint16 x3d_init(X3D_Context* context, X3D_Config* config) {
  uint16 dll_status;
  
  atexit(x3d_cleanup);
  
#ifdef __TIGCC__
  UnloadDLL();
  
  x3d_loaded = 0;
  
  if((dll_status = LoadDLL("x3d", X3D_ID, 1, 0)) != DLL_OK) {
    return dll_status;
  }
  
  // Set the error handler
  context->error_handler = x3d_default_error_handler;
  
  x3d_loaded = 1;
  x3d_init_core(context, config);
#endif
  
  return 0;
}
#endif