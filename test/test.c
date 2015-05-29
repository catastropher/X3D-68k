#define SAVE_SCREEN

#include <tigcclib.h>

#include <X3D/X3D.h>

int _ti92plus;

void _main() {
  clrscr();
  X3D_Vex3D_int16 a = { 5, 10, 15 };
  X3D_Vex3D_int16 b = { 10, 20, 30 };

  long dot = x3d_vex3d_int16_dot(&a, &b);

  printf("%ld\n", dot);
  ngetchx();
}