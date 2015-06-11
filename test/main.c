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

#define SAVE_SCREEN

#include <X3D/X3D.h>

#ifdef __TIGCC_HEADERS__

#include <tigcclib.h>

int _ti92plus;

#endif


#include <assert.h>


void test_vector();
void test_fastsqrt();

void run_tests() {
  // Add calls to test functions
  test_fastsqrt();
  test_vector();
}

void test_a();

#ifdef __TIGCC__
void _main() {
#else
int main() {
#endif

  X3D_STACK_TRACE;

  run_tests();

  x3d_printf("All tests passed! :D");
  
  ngetchx();
 
  test_a();

#ifndef __TIGCC__
  return 0;
#endif

}
