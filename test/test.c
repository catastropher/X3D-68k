#define SAVE_SCREEN

#include <tigcclib.h>

#include <X3D/X3D.h>

int _ti92plus;

void _main() {
  clrscr();

  char buf[100];

  x3d_sprintf(buf, "val = @fp8.8", 256 + 128 + 64);

  printf("%s", buf);

  ngetchx();
}
