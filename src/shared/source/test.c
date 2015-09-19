// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

// Everything in this file should be considered experimental!

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"
#include "X3D_clip.h"
#include "X3D_render.h"
#include "X3D_frustum.h"
#include "X3D_trig.h"
#include "X3D_segment.h"
#include "X3D_matrix.h"
#include "X3D_error.h"
#include "X3D_keys.h"
#include "X3D_engine.h"
#include "X3D_manager.h"
#include "X3D_collide.h"
#include "X3D_object.h"
#include "X3D_command.h"
#include "X3D_log.h"

#ifdef __TIGCC_HEADERS__
#include <tigcclib.h>
#endif

// Stuff to make visual studio shut up
#ifdef WIN32
#define INT_HANDLER int
#define GetIntVec(...) 0
#define DUMMY_HANDLER 0
#define SetIntVec(...) ;
#define FontSetSys(...) ;
#define LCD_WIDTH 240
#define LCD_HEIGHT 128
#define RR_LEFT 0,0
#define RR_RIGHT 0,0
#define RR_UP 0,0
#define RR_DOWN 0,0
#define RR_F1 0,0
#define RR_F2 0,0
#define RR_F5 0,0
#define RR_F7 0,0
#define RR_ESC 0,0
#define RR_Q 0,0
#define RR_W 0,0
#define RR_E 0,0
#define RR_R 0,0
#endif

enum {
  KEY_SCALE_UP = XKEY_CUSTOM1,
  KEY_SCALE_DOWN = XKEY_CUSTOM2,
  KEY_TRANSLATE_UP = XKEY_CUSTOM3,
  KEY_TRANSLATE_DOWN = XKEY_CUSTOM4,
  KEY_CYCLE_SEGMENT = XKEY_CUSTOM5,
  KEY_ADD_SEGMENT = XKEY_CUSTOM6,
  KEY_SWITCH_SEGMENT = XKEY_CUSTOM7
};

#if defined(__TIGCC__) || defined(WIN32) || 1


typedef struct X3D_TestContext {
  X3D_ViewPort context;
  X3D_EngineState state;
  X3D_RenderDevice device;
  X3D_KeyState keys;

  INT_HANDLER old_int_1;
  INT_HANDLER old_int_5;

  uint8 quit;
} X3D_TestContext;

void TEST_x3d_project_prism3d(X3D_Prism2D* dest, X3D_Prism3D* p, X3D_ViewPort* context) {
  uint16 i;
  for(i = 0; i < p->base_v * 2; ++i) {
    x3d_vex3d_int16_project(dest->v + i, p->v + i, context);
  }

  dest->base_v = p->base_v;
}

#define TICKS_PER_SECONDS 350

volatile uint16 hardware_timer;

DEFINE_INT_HANDLER(new_auto_int_1) {
  ++hardware_timer;
}

uint16 x3d_get_clock() {
  return hardware_timer;
}

static void x3d_test_init(X3D_TestContext* context, X3D_Context* c) {
  c->render_step = 0;
  c->frame = 0;
  
  x3d_init_segmentmanager(&c->segment_manager, 30, 20000);
  
  //x3d_enginestate_init(&context->state, 30, 20000);
  x3d_renderdevice_init(&context->device, LCD_WIDTH, LCD_HEIGHT);
  x3d_rendercontext_init(&context->context, context->device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  c->screen_data = context->device.dbuf;
  

  
  
  
  x3d_init_objectmanager(c);

  // Redirect interrupt handlers
  context->old_int_1 = GetIntVec(AUTO_INT_1);
  context->old_int_5 = GetIntVec(AUTO_INT_5);
  
  c->old_int_1 = context->old_int_1;
  c->old_int_5 = context->old_int_5;


  SetIntVec(AUTO_INT_1, new_auto_int_1);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

  x3d_keystate_map(&c->keys, XKEY_MAP_LEFT, RR_LEFT);
  x3d_keystate_map(&c->keys, XKEY_MAP_RIGHT, RR_RIGHT);
  x3d_keystate_map(&c->keys, XKEY_MAP_UP, RR_UP);
  x3d_keystate_map(&c->keys, XKEY_MAP_DOWN, RR_DOWN);
  x3d_keystate_map(&c->keys, XKEY_MAP_FORWARD, RR_F1);
  x3d_keystate_map(&c->keys, XKEY_MAP_BACK, RR_F2);
  x3d_keystate_map(&c->keys, XKEY_MAP_QUIT, RR_ESC);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM1, RR_Q);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM2, RR_W);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM3, RR_E);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM4, RR_R);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM5, RR_F5);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM6, RR_F7);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM7, RR_F4);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM8, RR_ESC);
  x3d_keystate_map(&c->keys, XKEY_MAP_CUSTOM9, RR_ESC);

  hardware_timer = 0;

  // We don't want to quit yet!
  c->quit = 0;
  
  //printf("Hello world!\n");
  //LCD_restore(c->screen_data);
  //while(1) ;
  
  x3d_debug(c, TRUE, "Init debug\n");
  
  //X3D_BlockAllocatorNode* node = c->object_manager.allocator.head;
}

static void x3d_test_copy_prism3d(X3D_Prism3D* dest, X3D_Prism3D* src) {
  dest->base_v = src->base_v;

  uint16 i;
  for(i = 0; i < src->base_v * 2; ++i) {
    dest->v[i] = src->v[i];
  }
}

void x3d_test_rotate_prism3d(X3D_Prism3D* dest, X3D_Prism3D* src, X3D_Camera* cam) {
  // Move the prism relative to the camera
  Vex3D cam_pos = { cam->object.pos.x >> 15, cam->object.pos.y >> 15, cam->object.pos.z >> 15 };
  
  // Account for the player's height
  cam_pos.y -= cam->object.volume.capsule.height;

  uint16 i;
  for(i = 0; i < src->base_v * 2; ++i) {
    Vex3D v = vex3d_int16_sub(src->v + i, &cam_pos);

    Vex3D temp;
    x3d_vex3d_int16_rotate(&temp, &v, &cam->object.mat);

    dest->v[i] = temp;
  }

  dest->base_v = src->base_v;
}

#define KEYSHIFT(_key, _shift) ((uint16)_keytest(_key) << (_shift))



void x3d_test_handle_keys(X3D_Context* context) {
  X3D_Camera* cam = context->cam;
  
  Vex3D_fp16x16 dir = { (int32)cam->object.mat.data[2] * 12, (int32)cam->object.mat.data[5] * 12, (int32)cam->object.mat.data[8] * 12};
  x3d_keystate_update(&context->keys);

  if(_keytest(RR_ENTER)) {
    while(_keytest(RR_ENTER)) ;
    
    x3d_enter_console(context);
  }
  
  

  if(_keytest(RR_1)) {
    cam->object.dir.x = 0;
    cam->object.dir.y = 0;
    cam->object.dir.z = 0;
  }  

  if(x3d_keystate_down(&context->keys, XKEY_FORWARD)) {
    //cam->object.pos.x += dir.x;
    //cam->object.pos.y += dir.y;
    //cam->object.pos.z += dir.z;

    cam->object.dir.z = dir.z;
    cam->object.dir.x = dir.x;
    
    //x3d_attempt_move_object(context, cam, NULL, 12);
  }
  else if (x3d_keystate_down(&context->keys, XKEY_BACK)) {
    cam->object.pos.x -= dir.x;
    cam->object.pos.y -= dir.y;
    cam->object.pos.z -= dir.z;
  }

  if (x3d_keystate_down(&context->keys, XKEY_UP)) {
    cam->object.angle.x += 3;
  }
  else if (x3d_keystate_down(&context->keys, XKEY_DOWN)) {
    cam->object.angle.x -= 3;
  }

  if (x3d_keystate_down(&context->keys, XKEY_LEFT)) {
    cam->object.angle.y -= 3;
  }
  else if (x3d_keystate_down(&context->keys, XKEY_RIGHT)) {
    cam->object.angle.y += 3;
  }
  if (x3d_keystate_down(&context->keys, XKEY_QUIT)) {
    context->quit = 1;
  }

  if (x3d_keystate_down_wait(&context->keys, KEY_CYCLE_SEGMENT)) {
    x3d_selectspinner_select(&context->spinner, context, context->spinner.selected_segment, context->spinner.selected_face + 1);
  }
  
  if(_keytest(RR_SPACE)) {
    cam->object.dir.y = -0xFFFFL * 3;
  
  }
  
  if (_keytest(RR_4)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism3D* prism = &s->prism;

    // Get the center of the prism
    Vex3D center;
    x3d_prism3d_get_center(prism, &center);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    
    Vex3D dir = { 0, -10, 0 };
    
    x3d_polygon3d_translate(poly, &dir);
    
    
    //x3d_move_polygon3d_along_normal(poly, 5, &center);
    x3d_prism3d_set_face(poly, prism, context->spinner.selected_face);
    
    x3d_calculate_segment_normals(s);

    free(poly);
  }

  if (x3d_keystate_down(&context->keys, KEY_TRANSLATE_UP)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism3D* prism = &s->prism;

    // Get the center of the prism
    Vex3D center;
    x3d_prism3d_get_center(prism, &center);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    x3d_move_polygon3d_along_normal(poly, 5, &center);
    x3d_prism3d_set_face(poly, prism, context->spinner.selected_face);
    
    x3d_calculate_segment_normals(s);

    free(poly);
  }

  if (x3d_keystate_down(&context->keys, KEY_TRANSLATE_DOWN)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism3D* prism = &s->prism;

    // Get the center of the prism
    Vex3D center;
    x3d_prism3d_get_center(prism, &center);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    x3d_move_polygon3d_along_normal(poly, -5, &center);
    x3d_prism3d_set_face(poly, prism, context->spinner.selected_face);

    x3d_calculate_segment_normals(s);
    
    free(poly);

  }

  if (x3d_keystate_down(&context->keys, KEY_SCALE_DOWN)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism3D* prism = &s->prism;

    // Get the center of the prism
    Vex3D center;
    x3d_prism3d_get_center(prism, &center);

    //X3D_LOG_WAIT(&context->context, "Center: %d, %d, %d", center.x, center.y, center.z);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    x3d_polygon3d_scale(poly, 225);
    x3d_prism3d_set_face(poly, prism, context->spinner.selected_face);
    
    x3d_calculate_segment_normals(s);

    free(poly);

  }

  if (x3d_keystate_down(&context->keys, KEY_SCALE_UP)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism3D* prism = &s->prism;

    // Get the center of the prism
    Vex3D center;
    x3d_prism3d_get_center(prism, &center);


    //X3D_LOG_WAIT(&context->context, "Center: %d, %d, %d", center.x, center.y, center.z);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    x3d_polygon3d_scale(poly, 256 + (256 - 225));
    x3d_prism3d_set_face(poly, prism, context->spinner.selected_face);
    
    x3d_calculate_segment_normals(s);

    free(poly);

  }

  if (x3d_keystate_down_wait(&context->keys, KEY_ADD_SEGMENT)) {
    X3D_Polygon3D* poly = malloc(sizeof(X3D_Polygon3D) + sizeof(Vex3D) * 30);

    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_Prism* prism = &s->prism;

    Vex3D center;
    x3d_prism3d_get_center(prism, &center);

    //X3D_LOG_WAIT(&context->context, "Center: %d, %d, %d", center.x, center.y, center.z);

    x3d_prism3d_get_face(poly, prism, context->spinner.selected_face);
    

    X3D_Segment* seg = x3d_segment_add(context, poly->total_v);

    X3D_SegmentFace* face = x3d_segment_get_face(s);

    face[context->spinner.selected_face].connect_id = seg->id;//x3d_get_total_segments(&context->state) - 1;
    
    X3D_SegmentFace* new_face = x3d_segment_get_face(seg);
    
    new_face[BASE_B].connect_id = s->id;

    //X3D_LOG_WAIT(&context->context, "Total: %d\n", poly->total_v);

    prism = &seg->prism;

    prism->base_v = poly->total_v;
    
    x3d_prism3d_set_face(poly, prism, BASE_B);
    x3d_move_polygon3d_along_normal(poly, -100, &center);
    x3d_prism3d_set_face(poly, prism, BASE_A);

    // Calculate the normals
    x3d_calculate_segment_normals(seg);

    context->spinner.selected_segment = seg->id;
    context->spinner.selected_face = BASE_A;

    free(poly);
  }
  
  if (x3d_keystate_down_wait(&context->keys, KEY_SWITCH_SEGMENT)) {
    X3D_Segment* s = x3d_get_segment(context, context->spinner.selected_segment);
    X3D_SegmentFace* face = x3d_segment_get_face(s);
    
    if(face[context->spinner.selected_face].connect_id != SEGMENT_NONE) {
      
      x3d_selectspinner_select(&context->spinner, context, face[context->spinner.selected_face].connect_id, BASE_A);
      
      //X3D_LOG_WAIT("New seg: %d\n", 
    }
    
  }
}

void x3d_test_cleanup(X3D_TestContext* context) {
  x3d_renderdevice_cleanup(&context->device);
  //x3d_enginestate_cleanup(&context->state);

  SetIntVec(AUTO_INT_1, context->old_int_1);
  SetIntVec(AUTO_INT_5, context->old_int_5);
}

uint16 bouncing_box;

void x3d_box_handler(X3D_Context* context, X3D_ObjectBase* obj, X3D_Event ev) {
  switch(ev.type) {
    case X3D_EV_CREATE:
      strcpy(context->status_bar, "Created!");
      break;
    case X3D_EV_RENDER:
    {
      X3D_Segment* box = x3d_get_segment(context, bouncing_box);
      X3D_Prism* temp = alloca(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * box->prism.base_v * 2);
      X3D_Prism* temp2 = alloca(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * box->prism.base_v * 2);
      uint16 i;
      
      Vex3D pos;
      
      x3d_object_pos(obj, &pos);
      
      for(i = 0; i < box->prism.base_v * 2; ++i) {
        temp2->v[i] = V3ADD(&box->prism.v[i], &pos);
      }
      
      temp2->base_v = box->prism.base_v;
    
      x3d_test_rotate_prism3d(temp, temp2, context->cam);
      x3d_draw_clipped_prism3d_wireframe(temp, ev.render.frustum, ev.render.viewport, 0, 0);
      
      x3d_attempt_move_object(context, obj, NULL , 10);
    }
      break;
  }
}

void x3d_cam_handler(X3D_Context* context, X3D_ObjectBase* obj, X3D_Event ev) {
  switch(ev.type) {
    case X3D_EV_RENDER:
      break;
  }
}

enum {
  OBJECT_CAM,
  OBJECT_BOX
};

void register_types(X3D_Context* context) {
  X3D_ObjectType box = {
    .event_handler = x3d_box_handler,
    .wall_behavior = X3D_COLLIDE_BOUNCE,
    .gravity = {0, 4000, 0}
  };

  box.volume.type = X3D_BOUND_SPHERE;
  box.volume.sphere.radius = 15;
  
  X3D_ObjectType cam = {
    .event_handler = x3d_cam_handler,
    .wall_behavior = X3D_COLLIDE_SLIDE,
    .gravity = {0, 17000, 0}
  };

  cam.volume.type = X3D_BOUND_CAPSULE;
  cam.volume.capsule.radius = 15;
  cam.volume.capsule.height = 40;
  
  x3d_add_object_type(context, OBJECT_BOX, &box);
  x3d_add_object_type(context, OBJECT_CAM, &cam);
}


void x3d_test() {
  X3D_TestContext test;

  FontSetSys(F_4x6);

  // Make some prisms
  
  X3D_Context context;
  
  x3d_test_init(&test, &context);
  

  X3D_Segment* seg = x3d_segment_add(&context, 4);
  
  register_types(&context);
  
   // Initialize the camera
  X3D_Camera* cam = (X3D_Camera*)x3d_create_object(&context, OBJECT_CAM, (Vex3D){ 0, 0, 0 }, (Vex3D_angle256){ 0, 0, 0 }, (Vex3D_fp0x16){ 0, 0, 0 }, FALSE, 0);
  cam->object.pos = (Vex3D_fp16x16){ 0, 0, 0 };
  cam->object.angle = (Vex3D_angle256){ 0, 0, 0 };
  
  context.cam = cam;
  
  //X3D_Segment* seg2 = x3d_segment_add(&test.state, 8);

  X3D_Prism* prism3d = &seg->prism;//malloc(sizeof(X3D_Prism3D) + sizeof(Vex3D) * 50 * 2);
  x3d_prism_construct(prism3d, 4, 200 * 3, 50 * 3, (Vex3D_uint8) { 0, 0, 0 });
  //x3d_prism_construct(&seg2->prism, 8, 200 * 3, 50 * 3, (Vex3D_uint8) { ANG_90, 0, 0 });

  //x3d_segment_get_face(seg)->connect_id = 1;

  //X3D_Prism* prism3d_rotated = malloc(sizeof(X3D_Prism3D) + sizeof(Vex3D) * 50 * 2);

  // Construct the viewing frustum
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 20);
  x3d_frustum_from_rendercontext(frustum, &test.context);

  //X3D_LOG_WAIT(&test.context, "DIFF: %ld\n", ((void *)seg) - ((void*)seg2));

  x3d_selectspinner_select(&context.spinner, &context, 0, 0);

  uint16 last_spin = x3d_get_clock();

  uint16 i;
  
  test.context.frame = 0;
  context.frame = 0;
  
  x3d_calculate_segment_normals(seg);

  for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
    cam->object.seg_pos.segs[i] = SEGMENT_NONE;
  }

  cam->object.seg_pos.segs[0] = 0;
  
  
  X3D_Segment* box =  x3d_segment_add(&context, 4);
  
  bouncing_box = box->id;
  
  x3d_prism_construct(&box->prism, 4, 20, 40, (Vex3D_uint8) { 0, 0, 0 });
  
  x3d_mat3x3_fp0x16_construct(&cam->object.mat, &cam->object.angle);
  
  
  
  cam->object.dir = (Vex3D_fp16x16) { 0, 0, 0 }; //dir;
  
  context.status_bar[0] = '\0';
  
  x3d_create_object(&context, OBJECT_BOX, (Vex3D){ 0, 0, 0 }, (Vex3D_angle256){ 0, 0, 0 }, (Vex3D_fp0x16){ 16384, -8192, 8192 }, FALSE, 0)->speed = 6;
  
  x3d_create_object(&context, OBJECT_BOX, (Vex3D){ 0, 0, 0 }, (Vex3D_angle256){ 0, 0, 0 }, (Vex3D_fp0x16){ -8192, -16384, 8192 }, FALSE, 0)->speed = 6;
  do {

    // Construct the rotation matrix
    x3d_mat3x3_fp0x16_construct(&cam->object.mat, &cam->object.angle);
    
    Vex3D_fp0x16 dir = { (int32)cam->object.mat.data[2], (int32)cam->object.mat.data[5], (int32)cam->object.mat.data[8]};

    clrscr();
    //x3d_draw_clipped_prism3d_wireframe(prism3d_rotated, frustum, &test.context);

    context.render_clock = 0;
    
    x3d_attempt_move_object(&context, (void *)cam, NULL, 10);

    //printf("%d\n", x3d_get_total_segments(&test.state));

    //for(i = 0; i < x3d_get_total_segments(&test.state); ++i) {
    //  x3d_render_segment_wireframe(i, frustum, &test.state, &test.context);
    //}

    for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
      if(cam->object.seg_pos.segs[i] != SEGMENT_NONE)
        x3d_render_segment_wireframe(cam->object.seg_pos.segs[i], frustum, &context, &test.context);
    }
    
    if(_keytest(RR_3)) {
      Vex3D pos;
      
      x3d_object_pos(cam, &pos);
      
      x3d_create_object(&context, OBJECT_BOX, pos , (Vex3D_angle256){ 0, 0, 0 }, dir, FALSE, 0)->speed = 6;
      
      while(_keytest(RR_3)) ;
    }
    

    printf("%s\n", context.status_bar);
    printf("%d\n", context.render_clock);

    //for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
    //  printf("%d ", cam->object.seg_pos.segs[i]);
    //}
    
    //printf("\n%d %d %d\n", cam->object.dir.x, cam->object.dir.y, cam->object.dir.z);
    
    Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    //printf("%ld %ld %ld\n", cam->object.dir.x, cam->object.dir.y, cam->object.dir.z);

    //printf("%d\n", x3d_point_in_segment(seg, &cam_pos));

    //printf("Face: %d\n", test.state.spinner.selected_face);

    if(context.spinner.selected_segment != SEGMENT_NONE && x3d_get_clock() - last_spin >= 75) {
      x3d_selectspinner_spin(&context.spinner);
      last_spin = x3d_get_clock();
    }
    
    ++context.frame;

    x3d_test_handle_keys(&context);

    x3d_renderdevice_flip(&test.device);

  } while(!context.quit);

  free(frustum);
  //free(prism3d);
  //free(prism3d_rotated);

  x3d_test_cleanup(&test);
}

#endif

