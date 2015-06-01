#define SAVE_SCREEN

#include <X3D/X3D.h>

#ifdef __TIGCC_HEADERS__

#include <tigcclib.h>

int _ti92plus;

#endif


#include <assert.h>


void test_vector();

void run_tests() {
  // Add calls to test functions
  test_vector();
}



#ifdef __TIGCC__
void _main() {
#else
int main() {
#endif

  run_tests();

  x3d_printf("All tests passed! :D");
  
  ngetchx();
 

#ifndef __TIGCC__
  return 0;
#endif

}
