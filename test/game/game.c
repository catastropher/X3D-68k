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

  X3D_Mat3x3_fp0x16 mat;

  clrscr();
  x3d_mat3x3_fp0x16_construct(&mat, 0, 0, 0);
  x3d_mat3x3_fp0x16_print(&mat);

  X3D_Vex3D_int16 in = { 50, 100, 150 };
  X3D_Vex3D_int16 out;

  x3d_vex3d_int16_rotate(&out, &in, &mat);
  x3d_vex3d_int16_print(&in);
  x3d_vex3d_int16_print(&out);

  ngetchx();

  X3D_RenderContext context;

  uint8* screen = malloc(LCD_SIZE);

  x3d_rendercontext_init(&context, screen, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);

  clrscr();

  X3D_Prism* prism = x3d_prism_construct(8, 25, 50, (X3D_Vex3D_angle256){ 0, 0, 0 });
  x3d_prism_render(prism, &context);

  free(prism);

  ngetchx();


}