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

#define USE_TI92PLUS
#define SAVE_SCREEN
#define EXECUTE_IN_GHOST_SPACE

#include <tigcclib.h>

#include "X3D_fix.h"
#include "init/X3D_init.h"
#include "X3D_engine.h"
#include "X3D_vector.h"

X3D_Context context;

void _main() {
  uint16 status;
  
  X3D_Config config;
  
  config.context_w = LCD_WIDTH;
  config.context_h = LCD_HEIGHT;
  config.screen_w = LCD_WIDTH;
  config.screen_h = LCD_HEIGHT;
  
  config.fov = 0;//ANG_60;
  config.max_segments = 20;
  config.seg_pool_size = 20000;
  config.flags = 0;
  
  config.log_flags = X3D_ENABLE_INFO | X3D_ENABLE_WARNING | X3D_ENABLE_ERROR | X3D_ENABLE_LOG_STDOUT | X3D_ENABLE_LOG_FILE;
  
  //config.context_pos = (Vex2D) { 0, 0 };
  
  if((status = x3d_init(&context, &config)) != 0) {
    clrscr();
    printf("Error loading DLL: %d\n", status);
    ngetchx();
  }
  
  ngetchx();
}

