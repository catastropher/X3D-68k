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

#include "X3D_platform.h"
#include "X3D_config.h"

#include "X3D_interface.h"

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

#define x3d_load_interface _DLL_call(void, (void), 0)

/**
 * Attempts to initialize X3D.
 */
_Bool x3d_init() {
  uint16 dll_status;
  const char* error_msg = "";
  
  x3d = &x3d_global_enginestate;
  
  // Step 1: init screen
  if(!x3d_platform_screen_init(&x3d->screen, LCD_WIDTH, LCD_HEIGHT, 1)) {
    error_msg = "Failed to init screen";
    goto error_screen;
  }
  
  // Step 2: load DLL
  UnloadDLL();
    
  if((dll_status = LoadDLL("x3d", X3D_ID, 1, 0)) != DLL_OK) {
    switch(dll_status) {
      case DLL_NOTFOUND:
        error_msg = "X3D DLL not found";
        break;
      default:
        break;
    }
        
    goto error_dll;
  }
  
  x3d_load_interface();	
  
  strcpy(x3d->error.msg, "Success!");
  
  //clrscr();
  //x3d->status.add("Hello, world!");
  //ngetchx();
  
  return X3D_FALSE;
  
error_dll:
  x3d_platform_screen_cleanup(&x3d->screen);
  
error_screen:

  strcpy(x3d->error.msg, error_msg);
  
  return X3D_FALSE;
}

