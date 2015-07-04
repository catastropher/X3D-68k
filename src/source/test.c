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

#ifdef __TIGCC_HEADERS__
#include <tigcclib.h>
#endif



#if defined(__TIGCC__) || defined(WIN32)

typedef X3D_Prism X3D_Prism3D;

typedef struct X3D_TestContext {
  X3D_RenderContext context;
  X3D_EngineState state;
  X3D_RenderDevice device;

  INT_HANDLER old_int_1;
  INT_HANDLER old_int_5;

  uint8 quit;
} X3D_TestContext;

void TEST_x3d_project_prism3d(X3D_Prism2D* dest, X3D_Prism3D* p, X3D_RenderContext* context) {
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

static void x3d_test_init(X3D_TestContext* context) {
  x3d_enginestate_init(&context->state, 5, 1000);
  x3d_renderdevice_init(&context->device, 240, 128);
  x3d_rendercontext_init(&context->context, context->device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  // Redirect interrupt handlers
  context->old_int_1 = GetIntVec(AUTO_INT_1);
  context->old_int_5 = GetIntVec(AUTO_INT_5);

  SetIntVec(AUTO_INT_1, new_auto_int_1);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

  hardware_timer = 0;

  // We don't want to quit yet!
  context->quit = 0;
}

static void x3d_test_copy_prism3d(X3D_Prism3D* dest, X3D_Prism3D* src) {
  dest->base_v = src->base_v;

  uint16 i;
  for(i = 0; i < src->base_v * 2; ++i) {
    dest->v[i] = src->v[i];
  }
}

// Stuff to make visual studio shut up
#ifdef WIN32
#define INT_HANDLER int
#define GetIntVec(...) 0
#define DUMMY_HANDLER 0
#define SetIntVec(...) ;
#define FontSetSys(...) ;
#define LCD_WIDTH 240
#define LCD_HEIGHT 128
#endif

void x3d_test_rotate_prism3d(X3D_Prism3D* dest, X3D_Prism3D* src, X3D_Camera* cam) {
  // Move the prism relative to the camera
  X3D_Vex3D_int16 cam_pos = { cam->pos.x >> 15, cam->pos.y >> 15, cam->pos.z >> 15 };

  uint16 i;
  for(i = 0; i < src->base_v * 2; ++i) {
    X3D_Vex3D_int16 v = vex3d_int16_sub(src->v + i, &cam_pos);

    X3D_Vex3D_int16 temp;
    x3d_vex3d_int16_rotate(&temp, &v, &cam->mat);

    dest->v[i] = temp;
  }

  dest->base_v = src->base_v;
}

void x3d_test_handle_keys(X3D_TestContext* context) {
  X3D_Camera* cam = &context->context.cam;
  X3D_Vex3D_int32 dir = { (int32)cam->mat.data[2] * 4, (int32)cam->mat.data[5] * 4, (int32)cam->mat.data[8] * 4 };

  if(_keytest(RR_F1)) {
    cam->pos.x += dir.x;
    cam->pos.y += dir.y;
    cam->pos.z += dir.z;
  }
  else if(_keytest(RR_F2)) {
    cam->pos.x -= dir.x;
    cam->pos.y -= dir.y;
    cam->pos.z -= dir.z;
  }

  if(_keytest(RR_UP)) {
    cam->angle.x++;
  }
  else if(_keytest(RR_DOWN)) {
    cam->angle.x--;
  }

  if(_keytest(RR_LEFT)) {
    cam->angle.y -= 1;
  }
  else if(_keytest(RR_RIGHT)) {
    cam->angle.y += 1;
  }
  if(_keytest(RR_ESC)) {
    context->quit = 1;
  }
}

void x3d_test_cleanup(X3D_TestContext* context) {
  x3d_renderdevice_cleanup(&context->device);
  x3d_enginestate_cleanup(&context->state);

  SetIntVec(AUTO_INT_1, context->old_int_1);
  SetIntVec(AUTO_INT_5, context->old_int_5);
}


void x3d_test() {
  X3D_TestContext test;
  x3d_test_init(&test);

  FontSetSys(F_4x6);

  // Initialize the camera
  X3D_Camera* cam = &test.context.cam;
  cam->pos = (X3D_Vex3D_fp16x16){ 0, 0, 0 };
  cam->angle = (X3D_Vex3D_angle256){ 0, 0, 0 };

  // Make some prisms

  X3D_Segment* seg = x3d_segment_add(&test.state, 50);

  X3D_Segment* seg2 = x3d_segment_add(&test.state, 8);

  X3D_Prism* prism3d = &seg->prism;//malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  x3d_prism_construct(prism3d, 8, 200 * 3, 50 * 3, (X3D_Vex3D_uint8) { 0, 0, 0 });
  x3d_prism_construct(&seg2->prism, 8, 200 * 3, 50 * 3, (X3D_Vex3D_uint8) { ANG_90, 0, 0 });

  x3d_segment_get_face(seg)->connect_id = 1;

  //X3D_Prism* prism3d_rotated = malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);

  // Construct the viewing frustum
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 20);
  x3d_frustum_from_rendercontext(frustum, &test.context);

  //X3D_LOG_WAIT(&test.context, "DIFF: %ld\n", ((void *)seg) - ((void*)seg2));

  if(seg2 == seg || ((void *)seg) - ((void*) seg2) != x3d_segment_needed_size(8)) {
    X3D_LOG_WAIT(&test.context, "ERROR");
  }

  do {
    // Construct the rotation matrix
    x3d_mat3x3_fp0x16_construct(&cam->mat, &cam->angle);

    clrscr();
    //x3d_draw_clipped_prism3d_wireframe(prism3d_rotated, frustum, &test.context);
    
    test.context.render_clock = 0;
    x3d_render_segment_wireframe(0, frustum, &test.state, &test.context);
    printf("Render clock: %d\n", test.context.render_clock);


    x3d_renderdevice_flip(&test.device);

    x3d_test_handle_keys(&test);
  } while(!test.quit);

  free(frustum);
  //free(prism3d);
  //free(prism3d_rotated);

  x3d_test_cleanup(&test);
}

#endif

