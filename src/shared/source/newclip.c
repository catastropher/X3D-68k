/* This file is part of X3D.
 * 
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#include "X3D_vector.h"

typedef struct X3D_BoundLine {
  Vex2D normal;
  Vex2D point;
} X3D_BoundLine;

void x3d_construct_boundline(X3D_BoundLine* line, Vex2D* a, Vex2D* b) {
  
}

typedef struct X3D_BoundRegion {
  uint16 total_bl;
  X3D_BoundLine line[];
} X3D_BoundRegion;


void x3d_construct_boundregion(X3D_BoundRegion* region, Vex2D v[], uint16 total_v) {
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    x3d_construct_boundline(region->line + i, v + i, v + (i % total_v));
  }
  
  region->total_bl = total_v;
}














































