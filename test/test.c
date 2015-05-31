#define SAVE_SCREEN

#include <tigcclib.h>

#include <X3D/X3D.h>

int _ti92plus;

#define EXPAND(_v) _v.x, _v.y, _v.z

void _main() {
  clrscr();

  char buf[100];

  X3D_Vex3D_int16 v = { 256, 128, 64 };

  x3d_sprintf(buf, "{@3,i16} + @i16 Name: @s\n", EXPAND(v), 5, "Michael");

  printf("%s", buf);

  ngetchx();
}
