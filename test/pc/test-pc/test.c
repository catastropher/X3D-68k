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
  KEY_DOWN = X3D_KEY_9,
  KEY_1 = X3D_KEY_10,
  KEY_2 = X3D_KEY_11,
  KEY_RECORD = X3D_KEY_12
};

void engine_test_handle_keys(void) {
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  static _Bool rec = X3D_FALSE;
  
  
  if(x3d_key_down(KEY_RECORD)) {
    while(x3d_key_down(KEY_RECORD)) {
      x3d_read_keys();
    }
    
    if(!rec) {    
      system("rm -rf ~/record");
      system("mkdir ~/record");
      
      printf("Begin recording in (hold M to abort):\n");
      
      _Bool record = X3D_TRUE;
      
      int16 i;
      for(i = 3; i >= 1; --i) {
        printf("%d\n", i);
        SDL_Delay(1000);
        
        x3d_read_keys();
        if(x3d_key_down(KEY_RECORD)) {
          printf("Recording aborted\n");
          while(x3d_key_down(KEY_RECORD)) {
            x3d_read_keys();
          }
          
          record = X3D_FALSE;
          break;
        }
      }
      
      if(record) {
        printf("Begin!\n");
        x3d_screen_begin_record("/home/michael/record/frame");
        rec = X3D_TRUE;
      }
    }
    else {
      printf("Recording complete\n");
      x3d_screen_record_end();
      rec = X3D_FALSE;
    }
  }
  
  if(x3d_key_down(KEY_1)) {
    X3D_RayCaster caster;
    X3D_Vex3D_fp0x16 dir;
    
    x3d_dynamicobject_forward_vector(&cam->base, &dir);
    
    x3d_raycaster_init(&caster, 0, cam->base.base.pos, dir);
    x3d_raycaster_cast(&caster);
    
    while(x3d_key_down(TEST_KEY_ENTER)) {
      x3d_read_keys();
    }
    
    printf("Seg: %d, Face: %d, Pos: { %d, %d, %d }\n", x3d_segfaceid_seg(caster.hit_face),
      x3d_segfaceid_face(caster.hit_face), caster.hit_pos.x, caster.hit_pos.y, caster.hit_pos.z);
    
    // Create a portal on one of the walls
    uint16 portal_base_v = 8;
    X3D_Polygon2D portal_poly = {
      .v = alloca(sizeof(X3D_Vex2D) * 20)
    };
    
    x3d_polygon2d_construct(&portal_poly, portal_base_v, 30, 0);
    x3d_wallportal_construct(0, caster.hit_face, caster.hit_pos, 1, &portal_poly, 5000);
    x3d_wallportal_update(0);
    x3d_wallportal_update(1);
  }
  
  if(x3d_key_down(KEY_2)) {
    X3D_RayCaster caster;
    X3D_Vex3D_fp0x16 dir;
    
    x3d_dynamicobject_forward_vector(&cam->base, &dir);
    
    x3d_raycaster_init(&caster, 0, cam->base.base.pos, dir);
    x3d_raycaster_cast(&caster);
    
    while(x3d_key_down(TEST_KEY_ENTER)) {
      x3d_read_keys();
    }
    
    printf("Seg: %d, Face: %d, Pos: { %d, %d, %d }\n", x3d_segfaceid_seg(caster.hit_face),
      x3d_segfaceid_face(caster.hit_face), caster.hit_pos.x, caster.hit_pos.y, caster.hit_pos.z);
    
    // Create a portal on one of the walls
    uint16 portal_base_v = 8;
    X3D_Polygon2D portal_poly = {
      .v = alloca(sizeof(X3D_Vex2D) * 20)
    };
    
    x3d_polygon2d_construct(&portal_poly, portal_base_v, 30, 0);
    x3d_wallportal_construct(1, caster.hit_face, caster.hit_pos, 0, &portal_poly, 31);
    x3d_wallportal_update(0);
    x3d_wallportal_update(1);
  }
  
  if(x3d_key_down(TEST_KEY_ESCAPE)) {
    x3d_game_loop_quit();
  }
  
  if(x3d_key_down(KEY_W)) {
    X3D_Vex3D dir;
    x3d_dynamicobject_forward_vector(&cam->base, &dir);

#if 0    
    X3D_Vex3D_fp8x8 d = {
      dir.x >> 5,
      dir.y >> 5,
      dir.z >> 5
    };
    
    cam->base.velocity = d;
    x3d_object_move(&cam->base);
#endif
    
    cam->base.base.pos.x += dir.x >> 5;
    cam->base.base.pos.y += dir.y >> 5;
    cam->base.base.pos.z += dir.z >> 5;
    
    //cam->base.base.pos.z += 4L << 8;
  }
  else if(x3d_key_down(KEY_S)) {
    X3D_Vex3D dir;
    x3d_dynamicobject_forward_vector(&cam->base, &dir);
    
    cam->base.base.pos.x -= dir.x >> 5;
    cam->base.base.pos.y -= dir.y >> 5;
    cam->base.base.pos.z -= dir.z >> 5;
    
    //cam->base.base.pos.z += 4L << 8;
  }
  
#if 1
  if(x3d_key_down(KEY_Q)) {
    --cam->base.angle.x;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
  else if(x3d_key_down(KEY_E)) {
    ++cam->base.angle.x;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
#endif
  
  if(x3d_key_down(KEY_A)) {
    --cam->base.angle.y;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
  else if(x3d_key_down(KEY_D)) {
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
  x3d_key_map_pc(KEY_1, '1');
  x3d_key_map_pc(KEY_2, '2');
  x3d_key_map_pc(KEY_RECORD, 'm');
  
  x3d_keymanager_set_callback(engine_test_handle_keys);
  
  // Create a new segment
  uint16 base_v = 8;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  
  x3d_prism3d_construct(prism, base_v, 200,  300, angle);
  uint16 id = x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
  
  //uint16 id2 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 1), 20);
  //uint16 id3 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id2, 3), 20);
  //uint16 id4 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id3, 1), 20);
  //uint16 id5 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 0), 20);
  //uint16 id6 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id5, 3), 20);
  uint16 id7 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 4), 100);
  uint16 id8 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id7, 4), 100);
  uint16 id9 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id7, 1), 450);
  uint16 id10 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id9, 1), 100);
  
  uint16 id11 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 1), 2000);
  
  // Create a portal on one of the walls
  uint16 portal_base_v = 8;
  X3D_Polygon2D portal_poly = {
    .v = alloca(sizeof(X3D_Vex3D) * 20)
  };
  
  x3d_polygon2d_construct(&portal_poly, portal_base_v, 30, 0);
  uint16 portal_green = x3d_wallportal_add(x3d_segfaceid_create(0, 3), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 5000);
  uint16 portal_red = x3d_wallportal_add(x3d_segfaceid_create(id, 7), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 31);
  
  x3d_wallportal_connect(portal_red, portal_green);
  x3d_wallportal_connect(portal_green, portal_red);
  
  
  //X3D_Color blue = x3d_rgb_to_color(0, 0, 255);
  //uint16 portal_blue = x3d_wallportal_add(x3d_segfaceid_create(0, 5), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, portal_poly, blue);
  
  //x3d_wallportal_connect(portal_blue, portal_red);
  
  
  
  // Setup the camera
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { 0, 0, 0 };
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

