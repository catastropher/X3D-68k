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

#pragma once

#include "X3D_vector.h"

//< Macro for giving 1D array index from a 3x3 matrix row and column
#define MAT3x3(_row, _col) ((_row) * 3 + (_col))

typedef struct X3D_Mat3x3_fp0x16 {
    fp0x16 data[9];
} X3D_Mat3x3_fp0x16;

typedef struct X3D_Mat4x4 {
    union {
        int32 elements[4][4];
        int32 elements1D[16];
    };
} X3D_Mat4x4;

typedef X3D_Mat3x3_fp0x16 X3D_Mat3x3;

struct X3D_CameraObject;

static inline void x3d_mat4x4_set_translation_column(X3D_Mat4x4* mat, X3D_Vex3D_fp16x16* src) {
    mat->elements[0][3] = src->x;
    mat->elements[1][3] = src->y;
    mat->elements[2][3] = src->z;
}

void x3d_mat3x3_mul(X3D_Mat3x3* dest, X3D_Mat3x3* a, X3D_Mat3x3* b);
void x3d_mat3x3_print(X3D_Mat3x3* mat);
void x3d_mat3x3_construct(X3D_Mat3x3 *dest, X3D_Vex3D_angle256 *angle);
void x3d_mat3x3_transpose(X3D_Mat3x3* mat);
void x3d_mat3x3_extract_angles(X3D_Mat3x3* mat, X3D_Vex3D_angle256* dest);
void x3d_mat3x3_visualize(X3D_Mat3x3* mat, X3D_Vex3D pos, struct X3D_CameraObject* cam);

void x3d_mat4x4_multiply(X3D_Mat4x4* a, X3D_Mat4x4* b, X3D_Mat4x4* dest);
void x3d_mat4x4_load_identity(X3D_Mat4x4* dest);
void x3d_mat4x4_load_translation_fp16x16(X3D_Mat4x4* dest, X3D_Vex3D_fp16x16* v);
void x3d_mat4x4_load_translation(X3D_Mat4x4* dest, X3D_Vex3D* v);

#include "math/X3D_matrix_inline.h"

