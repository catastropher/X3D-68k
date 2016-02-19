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
#include "X3D_vector.h"
#include "X3D_plane.h"
#include "X3D_matrix.h"

/**
* Constructs a plane from 3 points on the plane.
*
* @param p - plane
* @param a - first point
* @param b - middle point
* @param c - end point
*
* @return nothing
* @todo Fix formatting.
*/
void x3d_plane_construct(X3D_Plane* p, X3D_Vex3D_int16* a, X3D_Vex3D_int16* b, X3D_Vex3D_int16* c) {
  // Calculate the normal of the plane
  X3D_Vex3D v1 = x3d_vex3d_sub(a, b);
  X3D_Vex3D v2 = x3d_vex3d_sub(c, b);

  x3d_vex3d_fp0x16_cross(&p->normal, &v1, &v2);

#ifdef __68k__
  p->normal = x3d_vex3d_neg(&p->normal);
#endif

  // D = (AX + BY + CZ)
  p->d = x3d_vex3d_int16_dot(&p->normal, a) >> X3D_NORMAL_BITS;
}

void x3d_plane_print(X3D_Plane* p) {
  printf("X3D_Plane\n\tNormal: {%d, %d, %d}\n\tD: %d\n",
    p->normal.x, p->normal.y, p->normal.z, p->d);
}

#include <math.h>

_Bool x3d_plane_guess_orientation(X3D_Plane* plane, X3D_Mat3x3* dest, X3D_Vex3D* p) {
  X3D_Vex3D x, y, z = plane->normal;

#if 1
  if(plane->normal.z != 0) {
    float A = plane->normal.x / 32768.0;
    float B = plane->normal.y / 32768.0;
    float C = plane->normal.z / 32768.0;

    float D = plane->d;

    float u = 10000;
    
    if(plane->normal.z > 0)
      u = -u;
    
    float v = p->z - (-B * p->y + D - A * (p->x + u)) / C;

    float len = sqrt(u * u + v * v);

    u /= len;
    v /= len;

    x.x = u * 32767;
    x.z = -v * 32767;
    x.y = 0;

    x3d_vex3d_fp0x16_cross(&y, &z, &x);
    
    //y.x = 0;
    //y.z = 0;
    //y.y = 32767;
    
    x3d_log(X3D_INFO, "U: %f", u);
    x3d_log(X3D_INFO, "V: %f", v);
  }
  else {
    x.x = 32767;
    x.y = 0;
    x.z = 0;
    
    y.x = 0;
    y.y = 0;
    y.z = 32767;
    
    x3d_log(X3D_INFO, "Normal: %d", z.y);
    
  }

  
  /*if(plane->normal.z < 0) {
    x.x = -x.x;
    x.z = -x.z;
  }*/

  //x3d_vex3d_fp0x16_cross(&y, &x, &plane->normal);

  

#else
  if(plane->normal.z != 0) {
    int32 mul = 65536;
    int64 u = 1024 * mul;

    int64 nx = plane->normal.x;
    int64 ny = plane->normal.y;

    int64 axu = nx * p->x + ((nx * u) >> 15);
    int64 by = ny * p->y;
    int64 d = (int32)plane->d * 32768;

    int64 n = (by + d - axu);

    int64 v = p->z - (((int64)n) / plane->normal.z);

    x = (X3D_Vex3D) { u / mul, 0, v };

    x3d_vex3d_fp0x16_normalize(&x);

    x3d_log(X3D_INFO, "U: %d", x.x);
    x3d_log(X3D_INFO, "V: %d", x.z);

    x3d_vex3d_fp0x16_cross(&y, &x, &plane->normal);

    //x3d_log(X3D_INFO, "AXU: %ld", axu);
    //x3d_log(X3D_INFO, "by: %d", by);
    //x3d_log(X3D_INFO, "N: %d\n", n);
    //x3d_log(X3D_INFO, "D: %d\n", d);
  }
#endif

  x3d_mat3x3_set_column(dest, 0, &x);
  x3d_mat3x3_set_column(dest, 1, &y);
  x3d_mat3x3_set_column(dest, 2, &z);
  
  return X3D_TRUE;  
}
