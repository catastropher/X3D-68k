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
} X3D_TestContext;

void TEST_x3d_project_prism3d(X3D_Prism2D* dest, X3D_Prism3D* p, X3D_RenderContext* context) {
  uint16 i;
  for(i = 0; i < p->base_v * 2; ++i) {
    x3d_vex3d_int16_project(dest->v + i, p->v + i, context);
  }

  dest->base_v = p->base_v;
}

static void x3d_test_init_screen(X3D_TestContext* context) {
  x3d_enginestate_init(&context->state, 5, 1000);
  x3d_renderdevice_init(&context->device, 240, 128);

  x3d_rendercontext_init(&context->context, context->device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);
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



void x3d_test() {
  FontSetSys(F_4x6);
  
  clrscr();

  // Redirect interrupt handlers
  INT_HANDLER old_int_1 = GetIntVec(AUTO_INT_1);
  INT_HANDLER old_int_5 = GetIntVec(AUTO_INT_5);

  SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

  X3D_TestContext test;

  x3d_test_init_screen(&test);

  // Initialize the camera
  X3D_Camera* cam = &test.context.cam;
  cam->pos = (X3D_Vex3D_fp16x16){ 0, 0, 0 };
  cam->angle = (X3D_Vex3D_angle256){ 0, 0, 0 };

  // Allocate some prisms
  X3D_Prism* prism3d = malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  X3D_Prism* prism3d_temp = malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  X3D_Prism2D* prism2d = malloc(sizeof(X3D_Prism2D) + sizeof(X3D_Vex2D_int16) * 50 * 2);

  // Construct the viewing frustum
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 20);

  x3d_frustum_from_rendercontext(frustum, &test.context);


  x3d_prism_construct(prism3d, 8, 200 * 3, 50 * 3, (X3D_Vex3D_uint8){ 0, 0, 0 });

  X3D_Vex2D_int16 clip[4] = {
    { 30, LCD_HEIGHT - 20 },
    { LCD_WIDTH - 20, 0 },
    { LCD_WIDTH - 20, LCD_HEIGHT - 70 },
    { 30, LCD_HEIGHT - 1 }
  };


  //X3D_ClipRegion* r = x3d_construct_clipregion(clip, 4);
  clrscr();
  
  _Bool has_first = 0, has_second = 0;
  
  int16 cx = LCD_WIDTH / 2, cy = LCD_HEIGHT / 2;
  
  X3D_Vex2D first, second;
  int16 inside;

  do {
    x3d_mat3x3_fp0x16_construct(&cam->mat, &cam->angle);
    x3d_test_copy_prism3d(prism3d_temp, prism3d);

    prism3d_temp->draw_edges = 0xFFFFFFFF;

    // Move the prism relative to the camera
    X3D_Vex3D_int16 cam_pos = { cam->pos.x >> 15, cam->pos.y >> 15, cam->pos.z >> 15 };


    uint16 i;
    for(i = 0; i < prism3d_temp->base_v * 2; ++i) {
      prism3d_temp->v[i] = vex3d_int16_sub(prism3d_temp->v + i, &cam_pos);

      X3D_Vex3D_int16 temp;
      x3d_vex3d_int16_rotate(&temp, prism3d_temp->v + i, &cam->mat);

      prism3d_temp->v[i] = temp;
    }

    for(i = 0; i < prism3d->base_v * 2; ++i) {
      x3d_vex3d_int16_project(prism2d->v + i, prism3d_temp->v + i, &test.context);
    }

    prism2d->base_v = prism3d->base_v;

    clrscr();
    //x3d_prism_render(prism3d_temp, &test.context);

    //for(i = 0; i < 4; i++) {
    //  x3d_draw_line_black(&test.context, clip + i, clip + ((i + 1) % 4));
    //}

    //X3D_LOG_WAIT(&test.context, "Draw lines\n");

    //x3d_prism2d_clip(prism2d, r, &test.context);
    //clip_prism2d(prism2d, r, &test.context);

    x3d_draw_clipped_prism3d_wireframe(prism3d_temp, frustum);

    x3d_renderdevice_flip(&test.device);

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
      break;
    }



  } while(1);


done:


  x3d_renderdevice_cleanup(&test.device);
  x3d_enginestate_cleanup(&test.state);

  SetIntVec(AUTO_INT_1, old_int_1);
  SetIntVec(AUTO_INT_5, old_int_5);

  
}

#endif