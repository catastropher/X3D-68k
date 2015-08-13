#define USE_TI92PLUS
#define SAVE_SCREEN
#define EXECUTE_IN_GHOST_SPACE

#include <tigcclib.h>

#include "X3D.h"

void _main() {
  uint16 status;
  
  X3D_Context context;
  X3D_Config config;
  
  config.context_w = LCD_WIDTH;
  config.context_h = LCD_HEIGHT;
  config.screen_w = LCD_WIDTH;
  config.screen_h = LCD_HEIGHT;
  
  config.fov = 0;//ANG_60;
  config.max_segments = 20;
  config.seg_pool_size = 20000;
  config.flags = 0;
  
  //config.context_pos = (Vex2D) { 0, 0 };
  
  if((status = x3d_init(&context, &config)) != 0) {
    clrscr();
    printf("Error loading DLL: %d\n", status);
    ngetchx();
  }
}

