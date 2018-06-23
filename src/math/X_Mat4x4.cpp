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
#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"
#include "render/X_Palette.h"

void X_Mat4x4::loadIdentity()
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
    
    *this = identity;
}

void X_Mat4x4::loadXRotation(fp angle) 
{
    x_fp16x16 cosAngle = x_cos(angle).toFp16x16();
    x_fp16x16 sinAngle = x_sin(angle).toFp16x16();
    
    X_Mat4x4 xRotation = {
        {
            { X_FP16x16_ONE, 0, 0, 0 },
            { 0, cosAngle, -sinAngle, 0 },
            { 0, sinAngle, cosAngle, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *this = xRotation;
}

void X_Mat4x4::loadYRotation(fp angle)
{
    x_fp16x16 cosAngle = x_cos(angle).toFp16x16();
    x_fp16x16 sinAngle = x_sin(angle).toFp16x16();
    
    X_Mat4x4 yRotation = {
        {
            { cosAngle, 0, sinAngle, 0 },
            { 0, X_FP16x16_ONE, 0, 0 },
            { -sinAngle, 0, cosAngle, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *this = yRotation;
}

void X_Mat4x4::loadZRotation(fp angle)
{
    x_fp16x16 cosAngle = x_cos(angle).toFp16x16();
    x_fp16x16 sinAngle = x_sin(angle).toFp16x16();
    
    X_Mat4x4 zRotation = {
        {
            { cosAngle, -sinAngle, 0, 0 },
            { sinAngle, cosAngle, 0, 0 },
            { 0, 0, X_FP16x16_ONE, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *this = zRotation;
}

void X_Mat4x4::loadTranslation(const Vec3fp& translation)
{
    X_Mat4x4 translationMatrix = 
    {
        {
            { X_FP16x16_ONE, 0, 0, translation.x.toFp16x16() },
            { 0, X_FP16x16_ONE, 0, translation.y.toFp16x16() },
            { 0, 0, X_FP16x16_ONE, translation.z.toFp16x16() },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    *this = translationMatrix;
}

X_Mat4x4 X_Mat4x4::operator*(const X_Mat4x4& mat) const
{
    X_Mat4x4 res;

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            x_fp16x16 sum = 0;
            for(int k = 0; k < 4; ++k)
            {
                sum += ((long long)elem[i][k] * mat.elem[k][j]) >> 16;
            }
            
            res.elem[i][j] = sum;
        }
    }

    return res;
}

Vec416x16 X_Mat4x4::getColumn(int col) const
{
    return Vec416x16(elem[0][col], elem[1][col], elem[2][col], elem[3][col]);
}

void X_Mat4x4::setColumn(int col, const Vec416x16& v)
{
    elem[0][col] = v.x;
    elem[1][col] = v.y;
    elem[2][col] = v.z;
    elem[3][col] = v.w;
}

Vec416x16 X_Mat4x4::getRow(int row) const
{
    return Vec416x16(elem[row][0], elem[row][1], elem[row][2], elem[row][3]);
}

void X_Mat4x4::setRow(int row, const Vec416x16& v)
{
    elem[row][0] = v.x;
    elem[row][1] = v.y;
    elem[row][2] = v.z;
    elem[row][3] = v.w;
}

void x_mat4x4_transform_vec4(const X_Mat4x4* mat, const X_Vec4* src, X_Vec4_fp16x16* dest)
{
    x_fp16x16 res[4] = { 0 };
    const x_fp16x16* srcArray = &src->x;
    
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j) {
            res[i] += x_fp16x16_mul(mat->elem[i][j], srcArray[j]);
        }
    }
    
    *dest = x_vec4_make(res[0], res[1], res[2], res[3]);
}

void x_mat4x4_transform_vec3(const X_Mat4x4* mat, const Vec3* src, Vec3* dest)
{
    X_Vec4 vec4 = x_vec4_make(src->x, src->y, src->z, X_FP16x16_ONE);
    X_Vec4_fp16x16 res;
    x_mat4x4_transform_vec4(mat, &vec4, &res);
    
    dest->x = res.x;
    dest->y = res.y;
    dest->z = res.z;
    
    if(res.w != X_FP16x16_ONE)
        printf("ERROR %d\n", res.w);
    
    return;
    
    dest->x = res.x / res.w;
    dest->y = res.y / res.w;
    dest->z = res.z / res.w;
}

void x_mat4x4_rotate_normal(const X_Mat4x4* mat, const Vec3* normal, Vec3* dest)
{
    dest->x = x_fp16x16_mul(normal->x, mat->elem[0][0]) + x_fp16x16_mul(normal->y, mat->elem[0][1]) + x_fp16x16_mul(normal->z, mat->elem[0][2]);
    dest->y = x_fp16x16_mul(normal->x, mat->elem[1][0]) + x_fp16x16_mul(normal->y, mat->elem[1][1]) + x_fp16x16_mul(normal->z, mat->elem[1][2]);
    dest->z = x_fp16x16_mul(normal->x, mat->elem[2][0]) + x_fp16x16_mul(normal->y, mat->elem[2][1]) + x_fp16x16_mul(normal->z, mat->elem[2][2]);
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

void x_mat4x4_print_machine_readable(const X_Mat4x4* mat)
{
    printf("{");
    for(int i = 0; i < 4; ++i)
    {
        printf("{");
        for(int j = 0; j < 4; ++j)
        {
            printf("%f", x_fp16x16_to_float(mat->elem[i][j]));
            
            if(j != 3)
                printf(",");
        }
        printf("}");
        
        if(i != 3)
            printf(",");
    }
    
    printf("}\n");
}

void x_mat4x4_extract_view_vectors(const X_Mat4x4* mat, Vec3* forwardDest, Vec3* rightDest, Vec3* upDest)
{
    *rightDest = mat->getRow(0).toVec3();
    *upDest = -mat->getRow(1).toVec3();
    *forwardDest = mat->getRow(2).toVec3();
}

void x_mat4x4_invert_diagonal(const X_Mat4x4* mat, X_Mat4x4* dest)
{
    dest->loadIdentity();
    
    for(int i = 0; i < 4; ++i)
    {
        dest->elem[i][i] = x_fp16x16_div(X_FP16x16_ONE, mat->elem[i][i]);
    }
}

void x_mat4x4_transpose_3x3(X_Mat4x4* mat)
{
    for(int i = 1; i < 3; ++i)
    {
        for(int j = 0; j < i; ++j)
        {
            X_SWAP(mat->elem[i][j], mat->elem[j][i]);
        }
    }
}

void x_mat4x4_visualize(X_Mat4x4* mat, Vec3 position, X_RenderContext* renderContext)
{
    // const X_Palette* p = x_palette_get_quake_palette();
    // X_Color color[] = { p->brightRed, p->lightGreen, p->lightBlue };
    
    // for(int i = 0; i < 3; ++i)
    // {
    //     X_Vec4 v;
    //     x_mat4x4_get_column(mat, i, &v);
        
    //     Vec3 end = x_vec4_to_vec3(&v);
    //     end = x_vec3_scale_int(&end, 50);
    //     end += position;
        
    //     X_Ray3 r = x_ray3_make(position, end);
    //     x_ray3_render(&r, renderContext, color[i]);
    // }
}


