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


#include "X3D.h"

#if defined(__linux__)
#include <SDL2/SDL.h>
#include <alloca.h>
#endif

#if defined(__nspire__)
#include <SDL/SDL.h>
#include <alloca.h>
#endif

enum {
  KEY_WIREFRAME = X3D_KEY_0,
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
  KEY_9 = X3D_KEY_12,
  KEY_0 = X3D_KEY_13,  
  KEY_RECORD = X3D_KEY_14
};

void engine_test_handle_keys(void) {
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  static _Bool rec = X3D_FALSE;
  
  
  if(x3d_key_down(KEY_WIREFRAME)) {
    while(x3d_key_down(KEY_WIREFRAME))
      x3d_read_keys();
    
    x3d_rendermanager_get()->wireframe = !x3d_rendermanager_get()->wireframe;
  }

#if 0
  if(x3d_key_down(KEY_0)) {
    x3d_rendermanager_get()->near_z++;
    printf("Near z: %d\n", x3d_rendermanager_get()->near_z);
  }
  else if(x3d_key_down(KEY_9)) {
    x3d_rendermanager_get()->near_z--;
    printf("Near z: %d\n", x3d_rendermanager_get()->near_z);
  }
#endif
  
 
#ifdef __linux__
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
#endif
  
  if(x3d_key_down(KEY_1)) {
    X3D_RayCaster caster;
    X3D_Vex3D_fp0x16 dir;
    
    x3d_dynamicobject_forward_vector(&cam->base, &dir);
    
    x3d_raycaster_init(&caster, 0, cam->base.base.pos, dir);
    x3d_raycaster_cast(&caster);
    
    
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

#if 1     
    X3D_Vex3D_fp8x8 d = {
      dir.x >> 4,
      dir.y >> 4,
      dir.z >> 4
    };
    
    cam->base.velocity = d;
    x3d_object_move(&cam->base);
#else
    
    cam->base.base.pos.x += dir.x >> 5;
    cam->base.base.pos.y += dir.y >> 5;
    cam->base.base.pos.z += dir.z >> 5;
    
#endif
    
    //cam->base.base.pos.z += 4L << 8;
  }
  else if(x3d_key_down(KEY_S)) {
    X3D_Vex3D dir;
    x3d_dynamicobject_forward_vector(&cam->base, &dir);
    
    #if 1     
    X3D_Vex3D_fp8x8 d = {
      -dir.x >> 4,
      -dir.y >> 4,
      -dir.z >> 4
    };
    
    cam->base.velocity = d;
    x3d_object_move(&cam->base);
#else
    
    cam->base.base.pos.x += dir.x >> 5;
    cam->base.base.pos.y += dir.y >> 5;
    cam->base.base.pos.z += dir.z >> 5;
    
#endif
    
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

typedef struct X3D_BoxObject {
  X3D_DynamicObjectBase base;
  X3D_Prism3D* prism;
  _Bool move_up;
  int16 top;
  int16 bottom;
  angle256 angle;
} X3D_BoxObject;

void boxobject_event_handler(X3D_ObjectBase* object, X3D_ObjectEvent event) {
  X3D_BoxObject* box = (X3D_BoxObject* )object;
  uint16 i;
  
  X3D_Vex3D pos;
  x3d_object_pos(box, &pos);
  
  switch(event.type) {
    case X3D_OBJECT_EVENT_CREATE:
      box->angle = 0;
      box->move_up = X3D_TRUE;
      break;
      
    case X3D_OBJECT_EVENT_RENDER:
      x3d_prism3d_render_solid(box->prism, &pos, event.render_event.list, event.render_event.cam, 31, event.render_event.region);
      break;
      
    case X3D_OBJECT_EVENT_FRAME:
#if 1
      ++box->angle;
      //x3d_prism3d_construct(box->prism, 20, 50, 50, (X3D_Vex3D_angle256) { box->angle, box->angle, 0 });
      
      if(box->move_up) {
        if(pos.y > box->top) {
          box->base.base.pos.y -= 1L << 8;
        }
        else {
          box->move_up = X3D_FALSE;
        }
      }
      else {
        if(pos.y < box->bottom) {
          box->base.base.pos.y += 1L << 8;
        }
        else {
          box->move_up = X3D_TRUE;
        }
      }
#endif
      
    default:
      break;
  }
}

void polygon2d_add_x(X3D_Polygon2D* poly, int16 w, int16 h) {
  int16 s_x = w / 6;
  int16 s_y = h / 6;
 
#if 1
  x3d_polygon2d_add_point(poly, -w / 2, -h / 2);
  x3d_polygon2d_add_point(poly, -w / 2 + s_x, -h / 2);
  x3d_polygon2d_add_point(poly, 0, -s_y);
  x3d_polygon2d_add_point(poly, w / 2 - s_x, -h / 2);
  x3d_polygon2d_add_point(poly, w / 2, -h / 2);
  x3d_polygon2d_add_point(poly, w / 2, -h / 2 + s_y);
  x3d_polygon2d_add_point(poly, s_x / 2, s_y / 2);
  
  x3d_polygon2d_add_point(poly, w / 2, h / 2);
  x3d_polygon2d_add_point(poly, w / 2, h / 2 + s_y);
  x3d_polygon2d_add_point(poly, w / 2 - s_x, h / 2 + s_y);
  x3d_polygon2d_add_point(poly, 0, s_y + s_y);
  x3d_polygon2d_add_point(poly, -(w / 2 - s_x), h / 2 + s_y);
  x3d_polygon2d_add_point(poly, -w / 2, h / 2 + s_y);
  x3d_polygon2d_add_point(poly, -w / 2, h / 2);
  x3d_polygon2d_add_point(poly, -s_x / 2, s_y / 2);
  x3d_polygon2d_add_point(poly, -w / 2, -h / 2 + s_y);
  
#if 0
  x3d_polygon2d_add_point(poly, w / 2, h / 2 + s_y);
  x3d_polygon2d_add_point(poly, w / 2 - s_x , h / 2 + s_y);
  x3d_polygon2d_add_point(poly, 0, s_y);
  
  x3d_polygon2d_add_point(poly, -w / 2, -h / 2 + s_y);
#endif
#endif
}

#ifdef __68k__
#include <tigcclib.h>
#endif

void engine_test(void) {
  
#if defined(__linux__)
  X3D_InitSettings init = {
    .screen_w = 640,
    .screen_h = 480,
    .screen_scale = 1,
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
  };
#elif defined(__nspire__)
  X3D_InitSettings init = {
    .screen_w = 320,
    .screen_h = 240,
    .screen_scale = 1,
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
  };
#endif
  
  
  //clrscr();
  
  
  x3d_init(&init);
  
  x3d_screen_clear(0);
  x3d_screen_draw_line(0, 0, 30, 30, 1);
  x3d_screen_flip();
  
  //while(1) ;
  
  
  // Set up key mapping
#if defined(__linux__)
  x3d_key_map_pc(KEY_WIREFRAME, SDLK_RETURN);
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
  x3d_key_map_pc(KEY_0, '0');
  x3d_key_map_pc(KEY_9, '9');
  x3d_key_map_pc(KEY_RECORD, 'm');
  x3d_key_map_pc(X3D_KEY_15, 'p');
  
#elif defined(__nspire__)
  x3d_key_map_pc(KEY_W, SDLK_7);
  x3d_key_map_pc(KEY_S, SDLK_4);
  x3d_key_map_pc(KEY_A, SDLK_LEFT);
  x3d_key_map_pc(KEY_D, SDLK_RIGHT);
  x3d_key_map_pc(KEY_E, SDLK_UP);
  x3d_key_map_pc(KEY_Q, SDLK_DOWN);
  x3d_key_map_pc(TEST_KEY_ESCAPE, SDLK_ESCAPE);
  
#elif defined(__68k__)
  x3d_key_map_68k(KEY_Q, RR_UP);
  x3d_key_map_68k(KEY_A, RR_LEFT);
  x3d_key_map_68k(KEY_E, RR_DOWN);
  x3d_key_map_68k(KEY_D, RR_RIGHT);
  x3d_key_map_68k(KEY_W, RR_1);
  x3d_key_map_68k(KEY_S, RR_2);
  
#endif
  
  x3d_keymanager_set_callback(engine_test_handle_keys);
  
  // Create a new segment
  uint16 base_v = 8;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  
  x3d_prism3d_construct(prism, base_v, 400,  300, angle);
  
  x3d_rendermanager_get()->near_z = 10;
  x3d_rendermanager_get()->wireframe = X3D_FALSE;
  
  X3D_Polygon3D p = {
    .v = alloca(1000)
  };
  
  x3d_prism3d_get_face(prism, 0, &p);
  x3d_polygon3d_scale(&p, 256);
  x3d_prism3d_set_face(prism, 0, &p);
  
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
  
  uint16 id12 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, 0), 100);
  
  // Create a portal on one of the walls
  uint16 portal_base_v = 8;
  X3D_Polygon2D portal_poly = {
    .v = alloca(sizeof(X3D_Vex3D) * 20)
  };
  
  x3d_polygon2d_construct(&portal_poly, portal_base_v, 30, 0);
#if 0
  uint16 portal_green = x3d_wallportal_add(x3d_segfaceid_create(0, 3), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 5000);
  uint16 portal_red = x3d_wallportal_add(x3d_segfaceid_create(id, 7), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 31);
  
  x3d_wallportal_connect(portal_red, portal_green);
  x3d_wallportal_connect(portal_green, portal_red);
#endif
  
  //X3D_Color blue = x3d_rgb_to_color(0, 0, 255);
  //uint16 portal_blue = x3d_wallportal_add(x3d_segfaceid_create(0, 5), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, portal_poly, blue);
  
  //x3d_wallportal_connect(portal_blue, portal_red);
  
  
  
  
  
  
  X3D_ObjectType box_type = {
    .event_handler = boxobject_event_handler,
    .size = sizeof(X3D_BoxObject)
  };
  
  X3D_Prism3D* p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id12)))->prism;
  
  x3d_prism3d_get_face(p2, 1, &p);
  x3d_polygon3d_scale(&p, 64);
  x3d_prism3d_set_face(p2, 1, &p);
  
  
  uint16 id13 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id12, 1), 400);
  uint16 id14 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id13, 1), 200);
  
  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id14)))->prism;
  
  x3d_prism3d_get_face(p2, 1, &p);
  
  X3D_Vex3D center;
  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);
  
  
  //==================
  uint16 id15 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id14, 1), 200);
  
  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id15)))->prism;
  
  x3d_prism3d_get_face(p2, 1, &p);
  
  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);
  
  //==================
  uint16 id16 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id15, 1), 500);
  
  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id16)))->prism;
  
  x3d_prism3d_get_face(p2, 1, &p);
  
  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);
  
  
  X3D_Polygon2D texture = {
    .v = alloca(2000),
    .total_v = 0
  };
  
  X3D_Polygon3D texture3d = {
    .v = alloca(2000)
  };
  
  X3D_UncompressedSegment* ss = x3d_segmentmanager_get_internal(id);

  int16 face_id = 8;
  uint16 top_left_v = face_id - 2;
  uint16 bottom_right_v = ((top_left_v + 1) % ss->prism.base_v) + ss->prism.base_v;
  
  X3D_Mat3x3 mat;

  polygon2d_add_x(&texture, 125, 100);
  
  // Project the 2D polygon onto the wall
  
  X3D_Plane plane;
  x3d_prism3d_get_face(&ss->prism, face_id, &p);
  x3d_plane_construct(&plane, p.v, p.v + 1, p.v + 2);
  
  
  x3d_polygon2d_to_polygon3d(
    &texture,
    &texture3d,
    &plane,
    ss->prism.v + top_left_v,
    ss->prism.v + bottom_right_v,
    &mat
  );
  
  X3D_Handle h = x3d_handle_add(&texture3d);
  
  x3d_log(X3D_ERROR, "Handle: %d\n", h);
  
  x3d_uncompressedsegment_get_faces(ss)[face_id].texture = h;
  
#if 0  
  box->prism = box_prism;
  
  
  
  box->top = -100;
  box->bottom = 100;
#endif
  
  
  // Setup the camera
  x3d_camera_init();
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { 0, 0, 0 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  
  
#if 0
  X3D_Handle box_handle2 = x3d_object_create(1, (X3D_Vex3D) { 0, -50, 0 }, 0, (X3D_Vex3D) { 0, 0, 0 }, 0, (X3D_Vex3D_angle256) { 0, 0, 0 });
  X3D_BoxObject* box2 = x3d_handle_deref(box_handle2);
  
  box2->top = 150;
  box2->bottom = 1000;
  box2->prism = box_prism;
#endif
  
#if 0
  // Create a new box
  x3d_object_create_type(1, &box_type);
  X3D_Handle box_handle = x3d_object_create(1, (X3D_Vex3D) { 0, 0, 0 }, 0, (X3D_Vex3D) { 0, 0, 0 }, 0, (X3D_Vex3D_angle256) { 0, 0, 0 });
  X3D_BoxObject* box = x3d_handle_deref(box_handle);
  
  X3D_Prism3D* box_prism = alloca(x3d_prism3d_size(50));
  x3d_prism3d_construct(box_prism, 20, 50, 50, (X3D_Vex3D_angle256) { ANG_180, 0, 0 });
  
  x3d_prism3d_get_face(box_prism, 0, &p);
  x3d_polygon3d_scale(&p, 0);
  x3d_prism3d_set_face(box_prism, 0, &p);
  
  box->prism = box_prism;
#endif
  
  x3d_game_loop();
  
  x3d_cleanup();
}

typedef struct Test {
  const char* name;
  void (*run)(void);
} Test;

#ifdef __68k__

void SDL_Delay(int ms) {
  
}

#endif

void font_editor(void);

void x3d_clipregion_test();

#if defined(__linux__) || defined(__nspire__)
int main() {
#else
void _main() {
#endif

  //x3d_clipregion_test();
  //x3d_cleanup();

  
  //return 0;
  
  engine_test();
  return;
  
  
  x3d_log(X3D_INFO, "X3D manual tests for PC");
  
  Test tests[] = {
    {
      "Engine test",
      engine_test
    },
#if 0
    {
      "Font Editor",
      font_editor
    }
#endif
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

