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
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
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
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
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
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
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

void prism_test(void) {
  X3D_InitSettings init = {
    .screen_w = 1000,
    .screen_h = 1000,
    .screen_scale = 1,
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
  };
  
  x3d_init(&init);
  
  int16 base_v = 8  ;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  
  enum {
    KEY_QUIT = X3D_KEY_0,
    KEY_W = X3D_KEY_1,
    KEY_S = X3D_KEY_2,
    KEY_A = X3D_KEY_3,
    KEY_D = X3D_KEY_4,
    KEY_ENTER = X3D_KEY_5
  };
  
  x3d_key_map_pc(KEY_QUIT, SDLK_ESCAPE);
  x3d_key_map_pc(KEY_W, SDLK_w);
  x3d_key_map_pc(KEY_A, SDLK_a);
  x3d_key_map_pc(KEY_S, SDLK_s);
  x3d_key_map_pc(KEY_D, SDLK_d);
  x3d_key_map_pc(KEY_ENTER, SDLK_RETURN);
  
  
  X3D_CameraObject cam = {
    .base = {
      .base = {
        .pos = { 0, 0, (-200L) << 8 }
      }
    }
  };
  
  X3D_Vex3D_angle256 cam_angle = { 0, 0, 0 };
  x3d_mat3x3_construct(&cam.base.mat, &cam_angle);
  
  do {
    x3d_read_keys();
    x3d_prism3d_construct(prism, base_v, 100, 50, angle);
    
    x3d_screen_clear(0);
    x3d_prism3d_render(prism, &cam, 31);
    x3d_screen_flip();
    
    if(x3d_key_down(KEY_W)) {
      cam.base.base.pos.z += 4L << 8;
    }
    else if(x3d_key_down(KEY_S)) {
      cam.base.base.pos.z -= 4L << 8;
    }
    
    if(x3d_key_down(KEY_A)) {
      cam.base.base.pos.x -= 4L << 8;
    }
    else if(x3d_key_down(KEY_D)) {
      cam.base.base.pos.x += 4L << 8;
    }
    
    if(x3d_key_down(KEY_ENTER)) {
      X3D_Polygon3D* poly = alloca(x3d_polygon3d_size(base_v));
      x3d_prism3d_get_face(prism, X3D_BASE_A, poly);
      x3d_polygon3d_print(poly);

      X3D_Plane plane;
      x3d_plane_construct(&plane, poly->v, poly->v + 1, poly->v + 2);
      x3d_plane_print(&plane);
      
      x3d_prism3d_get_face(prism, X3D_BASE_B, poly);
      x3d_polygon3d_print(poly);
      
      x3d_plane_construct(&plane, poly->v, poly->v + 1, poly->v + 2);
      x3d_plane_print(&plane);
      
      x3d_prism3d_get_face(prism, 2, poly);
      x3d_polygon3d_print(poly);
      x3d_plane_construct(&plane, poly->v, poly->v + 1, poly->v + 2);
      x3d_plane_print(&plane);
      
      x3d_screen_clear(0);
      
      uint16 i;
      for(i = 0; i < base_v + 2; ++i) {
        x3d_prism3d_get_face(prism, i, poly);
        x3d_polygon3d_render_wireframe_no_clip(poly, &cam, 31);
        x3d_screen_flip();
        SDL_Delay(1000);
      }
      
      
      SDL_Delay(2000);
    }
  
    SDL_Delay(20);
    
    
    
    
    //angle.y++;
    //angle.x++;
  } while(!x3d_key_down(KEY_QUIT));
  
  x3d_cleanup();
}

enum {
  TEST_KEY_ENTER = X3D_KEY_0,
  KEY_W = X3D_KEY_1,
  KEY_S = X3D_KEY_2,
  KEY_A = X3D_KEY_3,
  KEY_D = X3D_KEY_4,
  TEST_KEY_ESCAPE = X3D_KEY_5,
  KEY_Q = X3D_KEY_6,
  KEY_E = X3D_KEY_7,
  KEY_UP = X3D_KEY_8,
  KEY_DOWN = X3D_KEY_9
};

void engine_test_handle_keys(void) {
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  
  if(x3d_key_down(TEST_KEY_ENTER)) {
    printf("Enter!\n");
    SDL_Delay(1000);
  }
  
  if(x3d_key_down(TEST_KEY_ESCAPE)) {
    x3d_game_loop_quit();
  }
  
  if(x3d_key_down(KEY_W)) {
      cam->base.base.pos.z += 4L << 8;
    }
    else if(x3d_key_down(KEY_S)) {
      cam->base.base.pos.z -= 4L << 8;
    }
    
    if(x3d_key_down(KEY_A)) {
      cam->base.base.pos.x -= 4L << 8;
    }
    else if(x3d_key_down(KEY_D)) {
      cam->base.base.pos.x += 4L << 8;
    }
    
    if(x3d_key_down(KEY_Q)) {
      --cam->base.angle.y;
      x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
    }
    else if(x3d_key_down(KEY_E)) {
      ++cam->base.angle.y;
      x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
    }
    
    if(x3d_key_down(KEY_UP)) {
      cam->base.base.pos.y -= 4L << 8;
    }
    else if(x3d_key_down(KEY_DOWN)) {
      cam->base.base.pos.y += 4L << 8;
    }
}

void engine_test(void) {
  X3D_InitSettings init = {
    .screen_w = 640,
    .screen_h = 480,
    .screen_scale = 1,
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
  };
  
  x3d_init(&init);
  
  // Set up key mapping
  x3d_key_map_pc(TEST_KEY_ENTER, SDLK_RETURN);
  x3d_key_map_pc(TEST_KEY_ESCAPE, SDLK_ESCAPE);
  x3d_key_map_pc(KEY_W, SDLK_w);
  x3d_key_map_pc(KEY_A, SDLK_a);
  x3d_key_map_pc(KEY_S, SDLK_s);
  x3d_key_map_pc(KEY_D, SDLK_d);
  x3d_key_map_pc(KEY_Q, SDLK_q);
  x3d_key_map_pc(KEY_E, SDLK_e);
  x3d_key_map_pc(KEY_UP, '[');
  x3d_key_map_pc(KEY_DOWN, ']');
  
  x3d_keymanager_set_callback(engine_test_handle_keys);
  
  // Create a new segment
  uint16 base_v = 8;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  
  x3d_prism3d_construct(prism, base_v, 50,  100, angle);
  uint16 id = x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
  
  uint16 id2 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 1), 20);
  uint16 id3 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id2, 3), 20);
  uint16 id4 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id3, 1), 20);
  uint16 id5 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 0), 20);
  uint16 id6 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id5, 3), 20);
  
  // Setup the camera
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { 0, 0, -300 << 8 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  
  
  x3d_game_loop();
  
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
    },
    {
      "Prism test",
      prism_test
    },
    {
      "Engine test",
      engine_test
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

