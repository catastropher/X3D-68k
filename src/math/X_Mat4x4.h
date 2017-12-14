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

#include "math/X_trig.h"
#include "geo/X_Vec3.h"
#include "geo/X_Vec4.h"

typedef struct X_Mat4x4
{
    x_fp16x16 elem[4][4];
} X_Mat4x4;

void x_mat4x4_load_identity(X_Mat4x4* mat);
void x_mat4x4_load_x_rotation(X_Mat4x4* mat, x_fp16x16 angle);
void x_mat4x4_load_y_rotation(X_Mat4x4* mat, x_fp16x16 angle);
void x_mat4x4_load_z_rotation(X_Mat4x4* mat, x_fp16x16 angle);
void x_mat4x4_load_translation(X_Mat4x4* mat, X_Vec3* translation);

void x_mat4x4_add(X_Mat4x4* a, X_Mat4x4* b, X_Mat4x4* dest);
void x_mat4x4_mul(const X_Mat4x4* a, const X_Mat4x4* b, X_Mat4x4* dest);
void x_mat4x4_transpose(X_Mat4x4* mat);

void x_mat4x4_get_column(const X_Mat4x4* mat, int col, X_Vec4* colDest);
void x_mat4x4_set_column(X_Mat4x4* mat, int col, const X_Vec4* colSrc);
void x_mat4x4_get_row(const X_Mat4x4* mat, int row, X_Vec4* rowDest);
void x_mat4x4_set_row(X_Mat4x4* mat, int row, const X_Vec4* rowSrc);

void x_mat4x4_transform_vec4(const X_Mat4x4* mat, const X_Vec4* src, X_Vec4_fp16x16* dest);
void x_mat4x4_transform_vec3(const X_Mat4x4* mat, const X_Vec3* src, X_Vec3* dest);

void x_mat4x4_rotate_normal(const X_Mat4x4* mat, const X_Vec3* normal, X_Vec3* dest);

void x_mat4x4_print(const X_Mat4x4* mat);
void x_mat4x4_print_machine_readable(const X_Mat4x4* mat);

void x_mat4x4_extract_view_vectors(const X_Mat4x4* mat, X_Vec3* forwardDest, X_Vec3* rightDest, X_Vec3* upDest);
void x_mat4x4_invert_diagonal(const X_Mat4x4* mat, X_Mat4x4* dest);
void x_mat4x4_transpose_3x3(X_Mat4x4* mat);

struct X_RenderContext;
void x_mat4x4_visualize(X_Mat4x4* mat, X_Vec3 position, struct X_RenderContext* renderContext);

