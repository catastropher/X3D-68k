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
#include <stddef.h>

#include "X3D.h"

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
  
  x3d_cleanup();
}

void key_test() {
  X3D_InitSettings init = {
    .screen_w = 256,
    .screen_h = 256,
    .screen_scale = 2,
    .fullscreen = X3D_FALSE
  };
  
  x3d_init(&init);
  
  x3d_log(X3D_INFO, "Running key test");
  x3d_log(X3D_INFO, "Enter=display message, Escape=quit\n");
  
  x3d_key_map_pc(X3D_KEY_0, SDLK_RETURN);
  x3d_key_map_pc(X3D_KEY_1, SDLK_ESCAPE);
  
  do {
    x3d_read_keys();
    
    if(x3d_key_down(X3D_KEY_0)) {
      x3d_log(X3D_INFO, "You pressed enter!");
      SDL_Delay(1000);
    }
  } while(!x3d_key_down(X3D_KEY_1));
  
  x3d_cleanup();
}

typedef struct FreeListBlock {
  int16 id;
  int16 pad0;
  void* safe_to_overwrite;
  int16 pad1;
  int16 pad2;
} FreeListBlock;

void freelist_test() {
  X3D_InitSettings init = {
    .screen_w = 256,
    .screen_h = 256,
    .screen_scale = 2,
    .fullscreen = X3D_FALSE
  };
  
  x3d_init(&init);
  
  X3D_FreeList list;
  FreeListBlock blocks[64];
  
  x3d_freelist_create(&list, blocks, sizeof(blocks), sizeof(FreeListBlock),
    offsetof(FreeListBlock, safe_to_overwrite),
    offsetof(FreeListBlock, id), 0);
  
  uint16 i;
  
  for(i = 0; i < 128; ++i) {
    FreeListBlock* block = x3d_freelist_alloc(&list);
    x3d_log(X3D_INFO, "Alloced block: %d", block->id);
    
    x3d_assert(((FreeListBlock *)x3d_freelist_get_block(&list, block->id))->id == block->id);
    
    x3d_freelist_free(&list, block);
  }
  
  FreeListBlock* block;
  
  for(i = 0; i < 64; ++i) {
    block = x3d_freelist_alloc(&list);
  }
  
  
  // Test when alloc pool is empty
  x3d_freelist_free(&list, block);
  block = x3d_freelist_alloc(&list);
  
  x3d_cleanup();
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
    },
    {
      "Key test",
      key_test
    },
    {
      "Freelist test",
      freelist_test
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

