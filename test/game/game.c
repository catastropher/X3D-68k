#define SAVE_SCREEN

#include <X3D/X3D.h>

#include <tigcclib.h>

#ifdef __TIGCC__

#include <tigcclib.h>

int _ti92plus;


#else

#define LCD_WIDTH 240
#define LCD_HEIGHT 127
#define LCD_SIZE (LCD_WIDTH * LCD_HEIGHT) / 8

#endif

//x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
//  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags)

void _main() {
  FontSetSys(F_4x6);

  X3D_RenderContext context;
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  int16 steps = 3;

  X3D_RenderDevice device;

  x3d_renderdevice_init(&device, 240, 128);

  x3d_rendercontext_init(&context, device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  clrscr();

  X3D_Prism* prism = malloc(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  
  INT_HANDLER old_int_1 = GetIntVec(AUTO_INT_1);
  INT_HANDLER old_int_5 = GetIntVec(AUTO_INT_5);

  SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

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

    clrscr();
    x3d_prism_construct(prism, steps, 25, 50, angle);
    x3d_prism_render(prism, &context);

    x3d_renderdevice_flip(&device);
  } while(1);

  free(prism);
  
  x3d_renderdevice_cleanup(&device);

  SetIntVec(AUTO_INT_1, old_int_1);
  SetIntVec(AUTO_INT_5, old_int_5);


}

