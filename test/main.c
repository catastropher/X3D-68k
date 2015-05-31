#define SAVE_SCREEN

#include <X3D/X3D.h>

#ifdef __TIGCC__

#include <tigcclib.h>

int _ti92plus;

#endif


#include <assert.h>


#ifdef __TIGCC__
void _main() {
#else
int main() {
#endif

  clrscr();
  x3d_printf("Hello world!\n");

  assert(sizeof(int16) != 2);

  x3d_printf("All tests passed! :D\n");
  ngetchx();

#ifndef __TIGCC__
  return 0;
#endif

}
