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

#define SAVE_SCREEN

#include <X3D/X3D.h>

#ifdef __TIGCC_HEADERS__

#include <tigcclib.h>

int _ti92plus;


#else

#define LCD_WIDTH 240
#define LCD_HEIGHT 127
#define LCD_SIZE (LCD_WIDTH * LCD_HEIGHT) / 8

#endif

//x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
//  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags)

void test_clip(X3D_RenderContext* context);

void x3d_prism2d_clip(X3D_Prism2D* prism, X3D_ClipRegion* clip, X3D_RenderContext* context);
X3D_ClipRegion* x3d_construct_clipregion(X3D_Vex2D_int16* v, uint16 total_v);


void test() {
  X3D_RenderContext context;

  x3d_rendercontext_init(&context, LCD_MEM, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);
  
  X3D_Prism* prism3d = malloc(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  X3D_Prism2D* prism2d = malloc(sizeof(X3D_Prism2D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  
  x3d_prism_construct(prism3d, 8, 25, 50, (X3D_Vex3D_uint8){0, 0, 0});
  
  uint16 i;
  for(i = 0; i < prism3d->base_v * 2; ++i) {
    x3d_vex3d_int16_project(prism2d->v + i, prism3d->v + i, &context);
  }
  
  prism2d->base_v = prism3d->base_v;
  
  X3D_Vex2D_int16 clip[4] = {
    { 0, 0 },
    { LCD_WIDTH - 1, 0 },
    { LCD_WIDTH - 1, LCD_HEIGHT - 1 },
    { 0, LCD_HEIGHT - 1 }
  };
  
  
  X3D_ClipRegion* r = x3d_construct_clipregion(clip, 4);
  clrscr();
  x3d_prism2d_clip(prism2d, r, &context);
  

  ngetchx();

  exit(-1);
}

void _main() {
  FontSetSys(F_4x6);
  
  
  //X3D_ClipRegion* region = x3d_construct_clipregion(clip, total_v);

  test();

  X3D_EngineState state;

  X3D_RenderContext context;
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  int16 steps = 3;

  X3D_RenderDevice device;

  x3d_enginestate_init(&state, 5, 1000);
  x3d_renderdevice_init(&device, 240, 128);

  x3d_rendercontext_init(&context, device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  clrscr();

  X3D_Frustum* f = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 5);

  X3D_Prism* prism = malloc(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * 50 * 2);

  INT_HANDLER old_int_1 = GetIntVec(AUTO_INT_1);
  INT_HANDLER old_int_5 = GetIntVec(AUTO_INT_5);

  SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

  x3d_frustum_from_rendercontext(f, &context);
  x3d_frustum_print(f);
  x3d_renderdevice_flip(&device);
  
  while(!_keytest(RR_ENTER));

  uint32 edges = 0xFFFFFFFF;

  do {
    if(_keytest(RR_UP)) {
      angle.x += 3;
    }
    if(_keytest(RR_DOWN)) {
      angle.x -= 3;
    }
    if(_keytest(RR_LEFT)) {
      angle.y -= 3;
    }
    if(_keytest(RR_RIGHT)) {
      angle.y += 3;
    }
    if(_keytest(RR_F1)) {
      if(steps > 1)
        --steps;

      while(_keytest(RR_F1));
    }
    if(_keytest(RR_F2)) {
      if(steps < 50)
        ++steps;

      while(_keytest(RR_F2));
    }
    if(_keytest(RR_ESC)) {
      break;
    }

    if(_keytest(RR_F3)) {
      while(_keytest(RR_F3));

      SetIntVec(AUTO_INT_1, old_int_1);
      SetIntVec(AUTO_INT_5, old_int_5);

      x3d_renderdevice_cleanup(&device);
      printf("Edge to toggle: ");
      char input[20];

      gets(input);

      edges ^= (1 << atoi(input));

      x3d_renderdevice_init(&device, 240, 128);

      SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
      SetIntVec(AUTO_INT_5, DUMMY_HANDLER);
    }

    clrscr();

    x3d_prism_construct(prism, steps, 25, 50, angle);
    prism->draw_edges = edges;
    //x3d_prism_render(prism, &context);
   

   

    x3d_renderdevice_flip(&device);
  } while(1);

  free(prism);

  x3d_renderdevice_cleanup(&device);
  x3d_enginestate_cleanup(&state);

  SetIntVec(AUTO_INT_1, old_int_1);
  SetIntVec(AUTO_INT_5, old_int_5);


}

