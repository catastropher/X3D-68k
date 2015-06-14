#define SAVE_SCREEN

#include <X3D/X3D.h>

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

  uint8* screen = malloc(LCD_SIZE);

  x3d_rendercontext_init(&context, screen, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  clrscr();

  X3D_Prism* prism = malloc(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * 32 * 2);
  
  void* kbd = kbd_queue();
  uint16 key;

  do {
    if(!OSdequeue(&key, kbd)) {
      if(key == KEY_UP) {
        angle.x += 3;
      }
      else if(key == KEY_DOWN) {
        angle.x -= 3;
      }
      else if(key == KEY_LEFT) {
        angle.y += 3;
      }
      else if(key == KEY_RIGHT) {
        angle.y -= 3;
      }
      else if(key == KEY_F1) {
        --steps;
      }
      else if(key == KEY_F2) {
        ++steps;
      }
      else if(key == KEY_ESC) {
        break;
      }
      
      clrscr();

      x3d_prism_construct(prism, steps, 25, 50, angle);
      x3d_prism_render(prism, &context);

    }
  } while(1);

  free(prism);

  ngetchx();


}