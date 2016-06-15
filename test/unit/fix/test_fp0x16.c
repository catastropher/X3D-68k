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

void test_fp0x16_should_be_at_least_2_bytes_big(void) {
  x3d_assert(sizeof(fp0x16) >= 2);
}

void test_fp0x16_should_correctly_convert_from_float(void) {
  x3d_assert(x3d_fp0x16_from_float(.5) == X3D_FP0x16_ONE_HALF);
}

void test_fp0x16_should_clamp_when_converting_from_positive_float_greater_than_or_eqaul_to_1(void) {
  x3d_assert(x3d_fp0x16_from_float(1.0) == X3D_FP0x16_MAX);
  x3d_assert(x3d_fp0x16_from_float(57.0) == X3D_FP0x16_MAX);
}

void test_fp0x16_should_clamp_when_converting_from_negative_float_less_than_minus_1(void) {
  x3d_assert(x3d_fp0x16_from_float(-1.7) == X3D_FP0x16_MIN);
}

void test_fp0x16_mul_by_fp0x16_as_fp0x16___should_get_correct_result(void) {
  {
    fp0x16 a = x3d_fp0x16_from_float(.5);
    fp0x16 b = X3D_FP0x16_ONE_HALF;
    fp0x16 result = x3d_fp0x16_mul_by_fp0x16_as_fp0x16(a, b);
    
    x3d_assert(result == X3D_FP0x16_ONE_FOURTH);
  }
  
  {
    fp0x16 a = -X3D_FP0x16_ONE_HALF;
    fp0x16 b = X3D_FP0x16_ONE_HALF;
    fp0x16 result = x3d_fp0x16_mul_by_fp0x16_as_fp0x16(a, b);
    
    x3d_assert(result == -X3D_FP0x16_ONE_FOURTH);
  }
}

void test_fp0x16_mul_by_int16_as_int16___should_get_correct_result(void) {
  fp0x16 a = X3D_FP0x16_ONE_HALF;
  int16  b = -1000;
  int16 result = x3d_fp0x16_mul_by_int16_as_int16(a, b);
  
  x3d_assert(result == -500);
}

void test_fp0x16_mul_by_int16_as_fp0x16___should_get_correct_result(void) {
  fp0x16 a = X3D_FP0x16_ONE_FOURTH;
  int16  b = 2;
  fp0x16 result = x3d_fp0x16_mul_by_int16_as_fp0x16(a, b);
  
  x3d_assert(result == X3D_FP0x16_ONE_HALF);
}

void test_fp0x16_div_by_fp0x16_as_fp16x16___should_get_correct_result(void) {
  fp0x16  a = x3d_fp0x16_from_float(.5);
  fp0x16  b = x3d_fp0x16_from_float(.025);
  fp16x16 result = x3d_fp0x16_div_by_fp0x16_as_fp16x16(a, b);
  
  x3d_assert(0);
}

void test_fp0x16(void) {
  test_fp0x16_should_be_at_least_2_bytes_big();
  
  test_fp0x16_should_correctly_convert_from_float();
  test_fp0x16_should_clamp_when_converting_from_positive_float_greater_than_or_eqaul_to_1();
  test_fp0x16_should_clamp_when_converting_from_negative_float_less_than_minus_1();
  
  test_fp0x16_mul_by_fp0x16_as_fp0x16___should_get_correct_result();
  test_fp0x16_mul_by_int16_as_int16___should_get_correct_result();
  test_fp0x16_mul_by_int16_as_fp0x16___should_get_correct_result();
  
  //test_fp0x16_div_by_fp0x16_as_fp16x16___should_get_correct_result();
}

