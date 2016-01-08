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

#include <stdio.h>

#include "X3D_common.h"
#include "X3D_polygon.h"

void x3d_polygon3d_print(X3D_Polygon3D* p) {
  printf("X3D_Polygon3D (v = %d)\n", p->total_v);
  
  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    printf("\t{%d, %d, %d}\n", p->v[i].x, p->v[i].y, p->v[i].z);
  }
  
  printf("\n");
}

