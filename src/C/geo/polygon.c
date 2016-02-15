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
#include "X3D_trig.h"
#include "X3D_matrix.h"

void x3d_polygon3d_print(X3D_Polygon3D* p) {
  printf("X3D_Polygon3D (v = %d)\n", p->total_v);
  
  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    printf("\t{%d, %d, %d}\n", p->v[i].x, p->v[i].y, p->v[i].z);
  }
  
  printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
/// Translates a 3D polygon.
///
/// @param poly - polygon to translate
/// @param dir  - direction to translate in
/// @param dist - distance to move polygon along dir
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist) {
  X3D_Vex3D shift = {
    ((int32)dist * dir->x) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->y) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->z) >> X3D_NORMAL_BITS
  };
  
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i].x += shift.x;
    poly->v[i].y += shift.y;
    poly->v[i].z += shift.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the center of a 3D polygon.
///
/// @param poly - poly
/// @param dest - dest for center of the polygon
///
/// @return Nothing.
/// @note This may lead to division by 0 if poly->total_v == 0
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_center(X3D_Polygon3D* poly, X3D_Vex3D* dest) {
  uint16 i;
  
  X3D_Vex3D_int32 center = { 0, 0, 0 };
  
  x3d_assert(poly->total_v != 0);
  
  for(i = 0; i < poly->total_v; ++i) {
    center.x += poly->v[i].x;
    center.y += poly->v[i].y;
    center.z += poly->v[i].z;
  }
  
  dest->x = center.x / poly->total_v;
  dest->y = center.y / poly->total_v;
  dest->z = center.z / poly->total_v;
}

///////////////////////////////////////////////////////////////////////////////
/// Scales a polygon relative to its center
///
/// @param poly - poly
/// @param scale  - scaling factor in fp8x8 format
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_scale(X3D_Polygon3D* poly, fp8x8 scale) {
  X3D_Vex3D center;
  x3d_polygon3d_center(poly, &center);
  
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i].x = (((int32)(poly->v[i].x - center.x) * scale) >> 8) + center.x;
    poly->v[i].y = (((int32)(poly->v[i].y - center.y) * scale) >> 8) + center.y;
    poly->v[i].z = (((int32)(poly->v[i].z - center.z) * scale) >> 8) + center.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Reverses the points in a 3D polygon. If it was clockwise, it will now be
///   counter-clockwise.
///
/// @param poly - poly to reverse
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_reverse(X3D_Polygon3D* poly) {
  uint16 i;
  
  for(i = 0; i < poly->total_v / 2; ++i) {
    X3D_SWAP(poly->v[i], poly->v[poly->total_v - i - 1]);
  }
}

/// @todo Document.
void x3d_polygon2d_construct(X3D_Polygon2D* poly, uint16 steps, int16 r, angle256 ang) {
  ufp8x8 angle = (uint16)ang << 8;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  poly->total_v = steps;

  // Construct the two bases (regular polygons)
  for(i = 0; i < steps; ++i) {
    poly->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cos(x3d_uint16_upper(angle)), r);
    poly->v[i].y = mul_fp0x16_by_int16_as_int16(x3d_sin(x3d_uint16_upper(angle)), r);
    
    angle += angle_step;
  }
}

void x3d_polygon2d_to_polygon3d(X3D_Polygon2D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, X3D_Vex3D* top_left, X3D_Vex3D* bottom_right, X3D_Mat3x3* mat) {
  X3D_Vex3D_fp0x16 x, y, z;
  
  X3D_Vex3D* v = top_left;
  
  z = plane->normal;
  
  // Calculate x axis
  x.y = 0;
  
  if(plane->normal.x != 0) {
    //x.z = 16384;
    

    float nx = plane->normal.x / 32767.0;
    float ny = plane->normal.y / 32767.0;
    float nz = plane->normal.z / 32767.0;
    
    
    
    float xz = .7; //top_left->z <= bottom_right->z ? 50 : -50;
    
    
    
    //float xx = v->x - ((plane->d - (plane->normal.y / 32767.0) * v->y - (plane->normal.z / 32767.0) * (
    //  v->z + xz)) / (plane->normal.x / 32767.0));
      
    float xx = (plane->d - ny * v->y - nz * (v->z + xz)) / nx - v->x;
    
    
    if(dest->total_v == 2) {
      //xx = .5;
      //xz = -.5;
    }
      
    
    
    x.x = bottom_right->x - top_left->x;
    x.z = bottom_right->z - top_left->z;
    
    //x.x = xx * 4096;
    //x.z = xz * 4096;

    
#if 0
    x.x = (x.x < 0 ? -x.x : x.x);
    
    if(top_left->x >= bottom_right->x)
      x.x = -x.x;
#endif
    
    
    //if(plane->normal.z > 0)
    //  x.z = -x.z;
    

    //if(plane->normal.x < 0)
    //  x.x = -x.x; 
    
    
    //int16 z_part = ((int32)plane->normal.z * ((int32)v->z + x.z)) >> 15;
    
    //x.x = (((int32)plane->d - (((int32)plane->normal.y * v->y) >> 15) - z_part) << 15) / plane->normal.x;
    
    //x.x -= v->x;
    
    x3d_vex3d_fp0x16_normalize(&x);
    
  }
  else {
    //x.x = 0;
    //x.z = 0x7FFF;
  }
  
  y = (X3D_Vex3D) { 0, 0x7FFF, 0 };
  
  
  mat->data[0] = x.x;
  mat->data[1] = x.y;
  mat->data[2] = x.z;
  
  mat->data[3] = y.x;
  mat->data[4] = y.y;
  mat->data[5] = y.z;
  
  mat->data[6] = z.x;
  mat->data[7] = z.y;
  mat->data[8] = z.z;
  
  dest->total_v = poly->total_v;
  
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    X3D_Vex3D vv = { poly->v[i].x, poly->v[i].y, 0 };
    
    //x3d_mat3x3_print(&mat);
    
    x3d_vex3d_int16_rotate(dest->v + i, &vv, mat);
    
    //dest->v[i].x += v->x;
    //dest->v[i].y += v->y;
    //dest->v[i].z += v->z;
  }
  
  
  //printf("%d %d %d\n", x.x, x.y, x.z);
}

void x3d_polygon3d_rotate(X3D_Polygon3D* poly, X3D_Vex3D_angle256 angle, X3D_Vex3D center) {
  X3D_Mat3x3 mat;
  x3d_mat3x3_construct(&mat, &angle);
  
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    X3D_Vex3D temp = x3d_vex3d_sub(poly->v + i, &center);
    x3d_vex3d_int16_rotate(poly->v + i, &temp, &mat);
    
    poly->v[i] = x3d_vex3d_add(poly->v + i, &center);
  }
}

void x3d_polygon2d_add_point(X3D_Polygon2D* poly, int16 x, int16 y) {
  poly->v[poly->total_v++] = (X3D_Vex2D) { x, y };
}

