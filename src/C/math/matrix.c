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

#include <string.h>
#include <stdio.h>

#include "X3D_common.h"
#include "X3D_matrix.h"
#include "X3D_vector.h"
#include "X3D_trig.h"
#include "X3D_fastsqrt.h"
#include "render/X3D_util.h"
#include "X3D_camera.h"

void x3d_mat4x4_multiply(X3D_Mat4x4* a, X3D_Mat4x4* b, X3D_Mat4x4* dest) {
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            fp16x16 sum = 0;
            
            for(int k = 0; j < 4; ++k) {
                sum += ((int64)a->elements[i][k] * b->elements[k][j]) >> 32;
            }
            
            dest->elements[i][j] = sum;
        }
    }
}

void x3d_mat4x4_load_identity(X3D_Mat4x4* dest) {
    const fp16x16 ONE = x3d_int16_to_fp16x16(1);
    const fp16x16 identity_matrix[4][4] = {
        { ONE,      0,      0,      0   },
        { 0,        ONE,    0,      0   },
        { 0,        0,      ONE,    0   },
        { 0,        0,      0,      ONE }
    };
    
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            dest->elements[i][j] = identity_matrix[i][j];
}

void x3d_mat4x4_load_translation_fp16x16(X3D_Mat4x4* dest, X3D_Vex3D_fp16x16* v) {
    x3d_mat4x4_load_identity(dest);
    x3d_mat4x4_set_translation_column(dest, v);
}

void x3d_mat4x4_load_translation(X3D_Mat4x4* dest, X3D_Vex3D* v) {
    X3D_Vex3D_fp16x16 v_as_fp16x16 = x3d_vex3d_to_vex3d_fp16x16(v);
    x3d_mat4x4_load_translation_fp16x16(dest, &v_as_fp16x16);
}

void x3d_mat4x4_transform_vex3d(X3D_Mat4x4* mat, X3D_Vex3D* v, X3D_Vex3D* dest) {
    int16 v4[4] = {
        v->x,
        v->y,
        v->z,
        1
    };
    
    fp16x16 result[4];
    
    for(int i = 0; i < 4; ++i) {
        result[i] = 0;
        
        for(int j = 0; j < 4; ++j) {
            result[i] += v4[j] * mat->elements[i][j];
        }
    }
    
    fp16x16 w = result[4];
    
    if(w == 0 || w == x3d_int16_to_fp16x16(1)) {
        dest->x = x3d_fp16x16_to_int16(result[0]);
        dest->y = x3d_fp16x16_to_int16(result[1]);
        dest->z = x3d_fp16x16_to_int16(result[2]);
        return;
    }
    
    dest->x = result[0] / w;
    dest->y = result[1] / w;
    dest->z = result[2] / w;
}



///////////////////////////////////////////////////////////////////////////////
/// Multiplies two fp0x16 matricies together (aka matrix concatenation).
///
/// @param dest - pointer to the destination matrix
/// @param a    - pointer to the first matrix to multiply
/// @param b    - pointer to the second matrix to multiply
///
/// @return nothing
/// @note dest must not be an alias for a or b
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_mul(X3D_Mat3x3* dest, X3D_Mat3x3* a, X3D_Mat3x3* b) {
  int i, j, k;

  for(i = 0; i < 9; i++)
    dest->data[i] = 0;

  for(i = 0; i < 3; i++) {
    for(j = 0; j < 3; j++) {
      for(k = 0; k < 3; k++) {
        dest->data[MAT3x3(i, j)] += x3d_fp0x16_mul(a->data[MAT3x3(i, k)], b->data[MAT3x3(k, j)]);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs an fp0x16 3D rotation matrix from a set of Euler anglers.
///
/// @param dest    - pointer to the destination matrix
/// @param angle   - pointer to Vex3D of angles describing rotation around
///    x, y, and z axis.
///
/// @return nothing
/// @todo Add support for rotation around Z axis.
/// @todo Construct matrix using product-to-sum identities instead of matrix
///    multiplication.
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_construct(X3D_Mat3x3_fp0x16 *dest, X3D_Vex3D_angle256 *angle) {
#if 1

  fp0x16 sin_y = x3d_sin(angle->y);
  fp0x16 cos_y = x3d_cos(angle->y);

  fp0x16 sin_x = x3d_sin(angle->x);
  fp0x16 cos_x = x3d_cos(angle->x);

  X3D_Mat3x3 mat_y = {
    {
      cos_y, 0, sin_y,
      0, INT16_MAX, 0,
      -sin_y, 0, cos_y
    }
  };

  X3D_Mat3x3 mat_x = {
    {
      INT16_MAX, 0, 0,
      0, cos_x, -sin_x,
      0, sin_x, cos_x,
    }
  };

  X3D_Mat3x3 mul_res;

  x3d_mat3x3_mul(&mul_res, &mat_y, &mat_x);

  memcpy(dest, &mul_res, sizeof(X3D_Mat3x3));
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// Prints out a 3x3 matrix.
///
/// @param mat  - the matrix to print out
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_print(X3D_Mat3x3* mat) {
  uint16 r;

  for(r = 0; r < 3; ++r) {
    printf("%d %d %d\n", mat->data[MAT3x3(r, 0)], mat->data[MAT3x3(r, 1)], mat->data[MAT3x3(r, 2)]);
  }
}

/// @todo Document.
void x3d_mat3x3_transpose(X3D_Mat3x3* mat) {
  X3D_Mat3x3 temp;

  uint16 i, j;

  for(i = 0; i < 3; ++i) {
    for(j = 0; j < 3; ++j) {
      temp.data[i * 3 + j] = mat->data[j * 3 + i];
    }
  }

  *mat = temp;
}

void x3d_mat3x3_extract_angles(X3D_Mat3x3* mat, X3D_Vex3D_angle256* dest) {
  int16 len = 2000;
  X3D_Vex3D v_before = { 0, 0, len };
  
  X3D_Vex3D v;
  x3d_vex3d_int16_rotate(&v, &v_before, mat);
  
  fp0x16 val = ((int32)v.y * 32767) / len;
  
  if(val < 0)
    dest->x = x3d_asin(val) - ANG_180;
  else
    dest->x = (angle256)x3d_acos(val) + ANG_270;
  
  dest->y = (angle256)ANG_90 - x3d_atan2(v.z, v.x);
}
