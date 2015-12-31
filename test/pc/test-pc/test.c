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
//
//
//
// test.c -> test for PC

#include <SDL2/SDL.h>

#include "X3D_common.h"
#include "X3D_log.h"
#include "X3D_init.h"
#include "X3D_screen.h"

void color_test(void) {
  X3D_InitSettings init = {
    .screen_w = 256,
    .screen_h = 256,
    .screen_scale = 2,
    .fullscreen = X3D_FALSE
  };
  
  x3d_init(&init);
  
  x3d_log(X3D_INFO, "Running color test");
  x3d_log(X3D_WARN, "PC uses 15-bit color, so gradients will not be completely smooth\n");
  
  x3d_log(X3D_INFO, "Clearing screen with shades of red");
  
  int16 i;
  for(i = 0; i < 256; ++i) {
    X3D_Color color = x3d_rgb_to_color(i, 0, 0);
    
    x3d_screen_clear(color);
    x3d_screen_flip();
    
    SDL_Delay(25);
  }
  
  x3d_log(X3D_INFO, "Clearing screen with shades of green");
  
  for(i = 0; i < 256; ++i) {
    X3D_Color color = x3d_rgb_to_color(0, i, 0);
    
    x3d_screen_clear(color);
    x3d_screen_flip();
    
    SDL_Delay(25);
  }
  
  x3d_log(X3D_INFO, "Clearing screen with shades of blue");
  
  for(i = 0; i < 256; ++i) {
    X3D_Color color = x3d_rgb_to_color(0, 0, i);
    
    x3d_screen_clear(color);
    x3d_screen_flip();
    
    SDL_Delay(25);
  }
  
  x3d_log(X3D_INFO, "Placing gradient pixel by pixel"); 
  
  int16 r, g, b;
  for(g = 0; g < 256; ++g) {
    for(r = 0; r < 256; ++r) {
      for(b = 0; b < 256; ++b) {
        X3D_Color color = x3d_rgb_to_color(r, g, b);
        
        x3d_screen_draw_pix(r, b, color);
      }
    }
    
    x3d_screen_flip();
    
    SDL_Delay(25);
  }
  
  x3d_log(X3D_INFO, "Finished running color test");

  SDL_Quit();
}

typedef struct Test {
  const char* name;
  void (*run)(void);
} Test;


int main() {
  x3d_log(X3D_INFO, "X3D manual tests for PC");
  
  Test tests[] = {
    {
      "Color test",
      color_test
    }
  };
  
  int total_tests = sizeof(tests) / sizeof(Test);
  _Bool done = X3D_FALSE;
  
  do {
    int16 i;
    
    printf("\n=============Select a Test to Run=============\n 0. Quit\n");
    
    for(i = 0; i < total_tests; ++i)
      printf("%2d. %s\n", i + 1, tests[i].name);
    
    _Bool valid = X3D_FALSE;
    int32 id;
    
    do {
      printf("\nTest: ");
      
      if(scanf("%d", &id) && id >= 0 && id <= total_tests)
        valid = X3D_TRUE;
      
    } while(!valid);
    
    if(id != 0)
      tests[id - 1].run();
    else
      done = X3D_TRUE;
    
  } while(!done);
}

