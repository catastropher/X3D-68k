// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <X3D/X3D.h>

void test_vector() {
  // Test case for x3d_vex3d_int16_add()
  {
    X3D_Vex3D_int16 a = { 1, 2, 3 };
    X3D_Vex3D_int16 b = { 2, 3, 4 };

    X3D_Vex3D_int16 res = vex3d_int16_add(&a, &b);

    x3d_assert(res.x == 3 && res.y == 5 && res.z == 7);
  }

  // Test case for x3d_vex3d_int16_sub()
  {
    X3D_Vex3D_int16 a = { 1, 2, 3 };
    X3D_Vex3D_int16 b = { 2, 3, 4 };

    X3D_Vex3D_int16 res = vex3d_int16_sub(&a, &b);

    x3d_assert(res.x == -1 && res.y == -1 && res.z == -1);
  }
  
  // Test case for x3d_vex3d_int16_dot()
  {
    X3D_Vex3D_int16 a = { 5, 10, 15 };
    X3D_Vex3D_int16 b = { 2, 3, 4 };

    int32 res = x3d_vex3d_int16_dot(&a, &b);

    x3d_assert(res == 10 + 30 + 60);
  }
  
  // Test case for x3d_vex3d_int16_dot()
  {
    X3D_Vex3D_int16 a = { 16384, 8192, 4096 };
    X3D_Vex3D_int16 b = { 4096, 2048, 1024 };

    int32 res = x3d_vex3d_fp0x16_dot(&a, &b);

    x3d_assert(res == 2688);
  }
  
  
}

