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

#include "X_Mat4x4.h"
#include "util/X_util.h"
#include "X_trig.h"

void x_mat4x4_load_identity(X_Mat4x4* mat)
{
    static const X_Mat4x4 identity =
    {
        {
            { X_FP16x16_ONE, 0, 0, 0 },
            { 0, X_FP16x16_ONE, 0, 0 },
            { 0, 0, X_FP16x16_ONE, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *mat = identity;
}

void x_mat4x4_load_x_rotation(X_Mat4x4* mat, x_angle256 angle) 
{
    x_fp16x16 cosAngle = x_cos(angle);
    x_fp16x16 sinAngle = x_sin(angle);
    
    X_Mat4x4 xRotation = {
        {
            { X_FP16x16_ONE, 0, 0, 0 },
            { 0, cosAngle, -sinAngle, 0 },
            { 0, sinAngle, cosAngle, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *mat = xRotation;
}

void x_mat4x4_load_y_rotation(X_Mat4x4* mat, x_angle256 angle) 
{
    x_fp16x16 cosAngle = x_cos(angle);
    x_fp16x16 sinAngle = x_sin(angle);
    
    X_Mat4x4 yRotation = {
        {
            { cosAngle, 0, sinAngle, 0 },
            { 0, X_FP16x16_ONE, 0, 0 },
            { -sinAngle, 0, cosAngle, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *mat = yRotation;
}

void x_mat4x4_load_z_rotation(X_Mat4x4* mat, x_angle256 angle) 
{
    x_fp16x16 cosAngle = x_cos(angle);
    x_fp16x16 sinAngle = x_sin(angle);
    
    X_Mat4x4 zRotation = {
        {
            { cosAngle, -sinAngle, 0, 0 },
            { sinAngle, cosAngle, 0, 0 },
            { 0, 0, X_FP16x16_ONE, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *mat = zRotation;
}

void x_mat4x4_load_translation(X_Mat4x4* mat, X_Vec3_fp16x16* translation)
{
    X_Mat4x4 translationMatrix = 
    {
        {
            { X_FP16x16_ONE, 0, 0, translation->x },
            { 0, X_FP16x16_ONE, 0, translation->y },
            { 0, 0, X_FP16x16_ONE, translation->z },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *mat = translationMatrix;
}

void x_mat4x4_add(X_Mat4x4* a, X_Mat4x4* b, X_Mat4x4* dest)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            dest->elem[i][j] = a->elem[i][j] + b->elem[i][j];
        }
    }
}

void x_mat4x4_mul(const X_Mat4x4* a, const X_Mat4x4* b, X_Mat4x4* dest)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            x_fp16x16 sum = 0;
            for(int k = 0; k < 4; ++k)
            {
                sum += ((long long)a->elem[i][k] * b->elem[k][j]) >> 16;
            }
            
            dest->elem[i][j] = sum;
        }
    }
}

void x_mat4x4_transpose(X_Mat4x4* mat)
{
    for(int i = 1; i < 4; ++i)
    {
        for(int j = 0; j < i; ++j)
        {
            X_SWAP(mat->elem[i][j], mat->elem[j][i]);
        }
    }
}

void x_mat4x4_get_column(const X_Mat4x4* mat, int col, X_Vec4* colDest)
{
    colDest->x = mat->elem[0][col];
    colDest->y = mat->elem[1][col];
    colDest->z = mat->elem[2][col];
    colDest->w = mat->elem[3][col];
}

void x_mat4x4_set_column(X_Mat4x4* mat, int col, const X_Vec4* colSrc)
{
    mat->elem[0][col] = colSrc->x;
    mat->elem[1][col] = colSrc->y;
    mat->elem[2][col] = colSrc->z;
    mat->elem[3][col] = colSrc->w;
}

void x_mat4x4_get_row(const X_Mat4x4* mat, int row, X_Vec4* rowDest)
{
    rowDest->x = mat->elem[row][0];
    rowDest->y = mat->elem[row][1];
    rowDest->z = mat->elem[row][2];
    rowDest->w = mat->elem[row][3];
}

void x_mat4x4_set_row(X_Mat4x4* mat, int row, const X_Vec4* rowSrc)
{
    mat->elem[row][0] = rowSrc->x;
    mat->elem[row][1] = rowSrc->y;
    mat->elem[row][2] = rowSrc->z;
    mat->elem[row][3] = rowSrc->w;
}

void x_mat4x4_transform_vec4(const X_Mat4x4* mat, const X_Vec4* src, X_Vec4_fp16x16* dest)
{
    x_fp16x16 res[4] = { 0 };
    const x_fp16x16* srcArray = &src->x;
    
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j) {
            res[i] += mat->elem[i][j] * srcArray[j];
        }
    }
    
    *dest = x_vec4_make(res[0], res[1], res[2], res[3]);
}

void x_mat4x4_transform_vec3(const X_Mat4x4* mat, const X_Vec3* src, X_Vec3* dest)
{
    X_Vec4 vec4 = x_vec4_make(src->x, src->y, src->z, 1);
    X_Vec4_fp16x16 res;
    x_mat4x4_transform_vec4(mat, &vec4, &res);
    
    dest->x = res.x / res.w;
    dest->y = res.y / res.w;
    dest->z = res.z / res.w;
}

void x_mat4x4_print(const X_Mat4x4* mat)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            printf("%f\t", x_fp16x16_to_float(mat->elem[i][j]));
        }
        
        printf("\n");
    }
    
    printf("\n");
}

void x_mat4x4_extract_view_vectors(const X_Mat4x4* mat, X_Vec3* forwardDest, X_Vec3* rightDest, X_Vec3* upDest)
{
    X_Vec4 right;
    x_mat4x4_get_row(mat, 0, &right);
    *rightDest = x_vec4_to_vec3(&right);
    
    X_Vec4 up;
    x_mat4x4_get_row(mat, 1, &up);
    *upDest = x_vec4_to_vec3(&up);
    *upDest = x_vec3_neg(upDest);
    
    X_Vec4 forward;
    x_mat4x4_get_row(mat, 2, &forward);
    *forwardDest = x_vec4_to_vec3(&forward);
}


