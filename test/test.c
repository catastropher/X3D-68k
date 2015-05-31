#define SAVE_SCREEN

#include <tigcclib.h>

#include <X3D/X3D.h>

int _ti92plus;

void _main() {
  clrscr();

  char buf[100];

  x3d_sprintf(buf, "total: @ui32, val = @fp8.8 (@fp0.16)", 5L, 256 + 128 + 64, 0x7FFF);

  printf("%s", buf);

  ngetchx();
}
