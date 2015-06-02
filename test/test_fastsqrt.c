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
#include <math.h>

// Average error: 1.5833322e-10
// Max error: 0.0625
void test_fastsqrt_accuracy() {
  uint32 i;
  
  double total = 0;
  double count = 0;
  
  double min_error = 1.0;
  double max_error = 0;
  
  uint32 max_error_input = 0;
  uint32 min_error_input = 0;
  
  for(i = 1; i < 0xFFFFFFFF; i++) {
    double error = ((double)(abs(x3d_fastsqrt(i) - sqrt(i))) / sqrt(i));
    
    total += error;
    ++count;
    
    if(error < min_error) {
      min_error = error;
      min_error_input = i;
    }
    
    if(error > max_error) {
      max_error = error;
      max_error_input = i;
    }
  }
  
  total;
  
  printf("Total error: %f\n", total);
  printf("Min error: %.20f (%ul)\n", min_error, min_error_input);
  
  printf("Max error: %.20f (%ul)\n", max_error, max_error_input);
}


void test_fastsqrt() {
  //test_fastsqrt_accuracy();
}

