// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D.h"

void test_vex3d_make___should_be_able_to_create_a_vector(void) {
  X3D_Vex3D v = x3d_vex3d_make(5, 10, 15);
  
  x3d_assert(v.x == 5);
  x3d_assert(v.y == 10);
  x3d_assert(v.z == 15);
}

void test_vex3d_equal___should_determine_when_vectors_are_equal(void) {
  X3D_Vex3D v = x3d_vex3d_make(1, 2, 3);
  x3d_assert(x3d_vex3d_equal(&v, &v));
}

void test_vex3d_equal___should_determine_when_vectors_are_not_equal(void) {
  X3D_Vex3D a = x3d_vex3d_make(1, 2, 3);
  X3D_Vex3D b = x3d_vex3d_make(2, 3, 4);
  
  x3d_assert(!x3d_vex3d_equal(&a, &b));
}

void test_vex3d_add___should_get_correct_result(void) {
  X3D_Vex3D a = x3d_vex3d_make(1, 2, 3);
  X3D_Vex3D b = x3d_vex3d_make(2, 3, 4);
  X3D_Vex3D result = x3d_vex3d_add(&a, &b);
  X3D_Vex3D correct_answer = x3d_vex3d_make(3, 5, 7);
  
  x3d_assert(x3d_vex3d_equal(&result, &correct_answer));
}

void test_x3d_vex3d_dot___should_get_correct_dot_product_for_small_vectors(void) {
  X3D_Vex3D a = x3d_vex3d_make(1, 2, 3);
  X3D_Vex3D b = x3d_vex3d_make(2, 3, 4);
  int32 result = x3d_vex3d_dot(&a, &b);
  int64 correct_answer = (1 * 2) + (2 * 3) + (3 * 4);
  
  x3d_assert(result == correct_answer);
}

void test_x3d_vex3d_dot___should_clamp_dot_product_for_large_vectors(void) {
  {
    X3D_Vex3D a = x3d_vex3d_make(0x7FFF, 0x7FFF, 0x7FFF);
    X3D_Vex3D b = x3d_vex3d_make(0x7FFF, 0x7FFF, 0x7FFF);
    int32 result = x3d_vex3d_dot(&a, &b);
    int32 correct_answer = INT32_MAX;
    
    x3d_assert(result == correct_answer);
  }
  
  {
    X3D_Vex3D a = x3d_vex3d_make(-0x7FFF, -0x7FFF, -0x7FFF);
    X3D_Vex3D b = x3d_vex3d_make(0x7FFF, 0x7FFF, 0x7FFF);
    int32 result = x3d_vex3d_dot(&a, &b);
    int32 correct_answer = INT32_MIN;
    
    x3d_assert(result == correct_answer);
  }
}

//void test_vex3d_fp0x16_dot___should_


void test_vector(void) {
  test_vex3d_make___should_be_able_to_create_a_vector();
  test_vex3d_add___should_get_correct_result();
  
  test_vex3d_equal___should_determine_when_vectors_are_equal();
  test_vex3d_equal___should_determine_when_vectors_are_not_equal();
  
  test_x3d_vex3d_dot___should_get_correct_dot_product_for_small_vectors();
  test_x3d_vex3d_dot___should_clamp_dot_product_for_large_vectors();
}

