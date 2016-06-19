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

void test_x3d_linear_interpolate(void) {
  {
    int16 start = -50;
    int16 end = 50;
    int16 result = x3d_linear_interpolate(start, end, X3D_FP0x16_ONE_HALF);
    int16 correct_answer = 0;
    x3d_assert(result == correct_answer);
  }
  
  {
    int16 start = 0;
    int16 end = 100;
    int16 result = x3d_linear_interpolate(start, end, X3D_FP0x16_ONE_FOURTH);
    int16 correct_answer = 25;
    x3d_assert(result == correct_answer);
  }
}

void x3d_ray3d_partially_in_front_of_near_plane___should_be_totally_in_front(void) {
  X3D_Ray3D ray = x3d_ray3d_make(
    x3d_vex3d_make(50, 50, 50),
    x3d_vex3d_make(100, 100, 100)
  );
  
  x3d_assert(x3d_ray3d_totally_in_front_of_near_plane(&ray));
}

void x3d_ray3d_partially_in_front_of_near_plane___should_not_be_partially_in_front(void) {
  X3D_Ray3D ray = x3d_ray3d_make(
    x3d_vex3d_make(-100, -100, -100),
    x3d_vex3d_make(-500, -500, -500)
  );
  
  x3d_assert(!x3d_ray3d_partially_in_front_of_near_plane(&ray));
}

void x3d_ray3d_partially_in_front_of_near_plane___should_be_at_least_partially_in_front(void) {
  X3D_Ray3D ray = x3d_ray3d_make(
    x3d_vex3d_make(-100, -100, -100),
    x3d_vex3d_make(500, 500, 500)
  );
  
  x3d_assert(x3d_ray3d_partially_in_front_of_near_plane(&ray));
}

void test_x3d_ray3d_interpolate(void) {
  X3D_Vex3D start = x3d_vex3d_make(-500, -500, -500);
  X3D_Vex3D end = x3d_vex3d_make(500, 500, 500);
  X3D_Ray3D ray = x3d_ray3d_make(start, end);
  
  X3D_Vex3D midpoint;
  x3d_ray3d_midpoint(&ray, &midpoint);
  
  {
    X3D_Vex3D result;
    x3d_ray3d_interpolate(&ray, 0, &result);
    x3d_assert(x3d_vex3d_equal(&result, &start));
  }
  
  {
    X3D_Vex3D result;
    x3d_ray3d_interpolate(&ray, X3D_FP0x16_ONE_HALF, &result);
    x3d_assert(x3d_vex3d_equal(&result, &midpoint));
  }
  
  {
    X3D_Vex3D result;
    x3d_ray3d_interpolate(&ray, 0x7FFF, &result);
    x3d_assert(x3d_vex3d_equal(&result, &end));
  }
  
}


void test_line(void) {
  test_x3d_linear_interpolate();
  
  x3d_ray3d_partially_in_front_of_near_plane___should_be_totally_in_front();
  x3d_ray3d_partially_in_front_of_near_plane___should_not_be_partially_in_front();
  x3d_ray3d_partially_in_front_of_near_plane___should_be_at_least_partially_in_front();
  
  test_x3d_ray3d_interpolate();
}


