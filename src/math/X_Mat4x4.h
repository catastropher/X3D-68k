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

struct X_Mat4x4
{
    void loadIdentity();
    void loadXRotation(fp angle);
    void loadYRotation(fp angle);
    void loadZRotation(fp angle);
    void loadTranslation(const Vec3fp& translation);

    Vec416x16 getColumn(int col) const;
    Vec416x16 getRow(int row) const;

    void setColumn(int col, const Vec416x16& v);
    void setRow(int col, const Vec416x16& v);

    X_Mat4x4 operator*(const X_Mat4x4& mat) const;

    x_fp16x16 elem[4][4];
};

void x_mat4x4_transform_vec4(const X_Mat4x4* mat, const X_Vec4* src, X_Vec4_fp16x16* dest);
void x_mat4x4_transform_vec3(const X_Mat4x4* mat, const Vec3* src, Vec3* dest);

void x_mat4x4_rotate_normal(const X_Mat4x4* mat, const Vec3* normal, Vec3* dest);

void x_mat4x4_print(const X_Mat4x4* mat);
void x_mat4x4_print_machine_readable(const X_Mat4x4* mat);

void x_mat4x4_extract_view_vectors(const X_Mat4x4* mat, Vec3* forwardDest, Vec3* rightDest, Vec3* upDest);
void x_mat4x4_invert_diagonal(const X_Mat4x4* mat, X_Mat4x4* dest);
void x_mat4x4_transpose_3x3(X_Mat4x4* mat);

struct X_RenderContext;
void x_mat4x4_visualize(X_Mat4x4* mat, Vec3 position, struct X_RenderContext* renderContext);

