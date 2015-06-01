#include <X3D/X3D.h>

void test_vector() {
  // Test case for x3d_vex3d_int16_add()
  {
    X3D_Vex3D_int16 a = { 1, 2, 3 };
    X3D_Vex3D_int16 b = { 2, 3, 4 };

    X3D_Vex3D_int16 res = x3d_vex3d_int16_add(&a, &b);

    x3d_assert(res.x == 3 && res.y == 5 && res.z == 7);
  }

  // Test case for x3d_vex3d_int16_sub()
  {
    X3D_Vex3D_int16 a = { 1, 2, 3 };
    X3D_Vex3D_int16 b = { 2, 3, 4 };

    X3D_Vex3D_int16 res = x3d_vex3d_int16_sub(&a, &b);

    x3d_assert(res.x == -1 && res.y == -1 && res.z == -1);
  }
}