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

#include "Mat4x4.hpp"
#include "util/Util.hpp"
#include "Trig.hpp"
#include "render/RenderContext.hpp"
#include "geo/Ray3.hpp"
#include "render/Palette.hpp"

void Mat4x4::loadIdentity()
{
    static const Mat4x4 identity =
    {
        {
            { 1.0_fp, 0, 0, 0 },
            { 0, 1.0_fp, 0, 0 },
            { 0, 0, 1.0_fp, 0 },
            { 0, 0, 0, 1.0_fp }
        }
    };
    
    *this = identity;
}

void Mat4x4::loadXRotation(fp angle) 
{
    fp cosAngle = x_cos(angle);
    fp sinAngle = x_sin(angle);
    
    Mat4x4 xRotation = {
        {
            { 1.0_fp, 0, 0, 0 },
            { 0, cosAngle, -sinAngle, 0 },
            { 0, sinAngle, cosAngle, 0 },
            { 0, 0, 0, 1.0_fp }
        }
    };
    
    *this = xRotation;
}

void Mat4x4::loadYRotation(fp angle)
{
    fp cosAngle = x_cos(angle);
    fp sinAngle = x_sin(angle);
    
    Mat4x4 yRotation = {
        {
            { cosAngle, 0, sinAngle, 0 },
            { 0, 1.0_fp, 0, 0 },
            { -sinAngle, 0, cosAngle, 0 },
            { 0, 0, 0, 1.0_fp }
        }
    };
    
    *this = yRotation;
}

void Mat4x4::loadZRotation(fp angle)
{
    x_fp16x16 cosAngle = x_cos(angle).toFp16x16();
    x_fp16x16 sinAngle = x_sin(angle).toFp16x16();
    
    Mat4x4 zRotation = {
        {
            { cosAngle, -sinAngle, 0, 0 },
            { sinAngle, cosAngle, 0, 0 },
            { 0, 0, 1.0_fp, 0 },
            { 0, 0, 0, 1.0_fp }
        }
    };
    
    *this = zRotation;
}

void Mat4x4::loadTranslation(const Vec3fp& translation)
{
    Mat4x4 translationMatrix = 
    {
        {
            { 1.0_fp, 0, 0, translation.x },
            { 0, 1.0_fp, 0, translation.y },
            { 0, 0, 1.0_fp, translation.z },
            { 0, 0, 0, 1.0_fp }
        }
    };
    
    *this = translationMatrix;
}

Mat4x4 Mat4x4::operator*(const Mat4x4& mat) const
{
    Mat4x4 res;

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            fp sum = 0;
            for(int k = 0; k < 4; ++k)
            {
                sum += elem[i][k] * mat.elem[k][j];
            }
            
            res.elem[i][j] = sum;
        }
    }

    return res;
}

Vec4 Mat4x4::getColumn(int col) const
{
    return Vec4(elem[0][col], elem[1][col], elem[2][col], elem[3][col]);
}

void Mat4x4::setColumn(int col, const Vec4& v)
{
    elem[0][col] = v.x;
    elem[1][col] = v.y;
    elem[2][col] = v.z;
    elem[3][col] = v.w;
}

Vec4 Mat4x4::getRow(int row) const
{
    return Vec4(elem[row][0], elem[row][1], elem[row][2], elem[row][3]);
}

void Mat4x4::setRow(int row, const Vec4& v)
{
    elem[row][0] = v.x;
    elem[row][1] = v.y;
    elem[row][2] = v.z;
    elem[row][3] = v.w;
}

Vec4 Mat4x4::transform(const Vec4& src) const
{
    fp res[4] = { 0, 0, 0, 0 };
    const fp* srcArray = &src.x;
    
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            res[i] += elem[i][j] * srcArray[j];
        }
    }

    return Vec4(res[0], res[1], res[2], res[3]);
}

Vec3fp Mat4x4::transform(const Vec3fp& src) const
{
    Vec4 v = Vec4(src.x, src.y, src.z, 1.0_fp);
    Vec4 transformed = transform(v);

    return transformed.toVec3();
    
    if(transformed.w != X_FP16x16_ONE)
        printf("ERROR %d\n", transformed.w);
    
    // dest->x = res.x / res.w;
    // dest->y = res.y / res.w;
    // dest->z = res.z / res.w;
}

Vec3fp Mat4x4::transformNormal(const Vec3fp& normal) const
{
    Vec3 norm = MakeVec3(normal);

    return Vec3fp(
        normal.x * elem[0][0] + normal.y * elem[0][1] + normal.z * elem[0][2],
        normal.x * elem[1][0] + normal.y * elem[1][1] + normal.z * elem[1][2],
        normal.x * elem[2][0] + normal.y * elem[2][1] + normal.z * elem[2][2]);
}

void Mat4x4::print() const
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            printf("%f\t", elem[i][j].toFloat());
        }
        
        printf("\n");
    }
    
    printf("\n");
}

void Mat4x4::extractViewVectors(Vec3fp& forwardDest, Vec3fp& rightDest, Vec3fp& upDest) const
{
    rightDest = getRow(0).toVec3();
    upDest = -getRow(1).toVec3();
    forwardDest = getRow(2).toVec3();
}

void Mat4x4::invertDiagonal(Mat4x4 dest) const
{
    dest.loadIdentity();
    
    for(int i = 0; i < 4; ++i)
    {
        dest.elem[i][i] = fp::fromInt(1) / elem[i][i];
    }
}

void Mat4x4::transpose3x3()
{
    for(int i = 1; i < 3; ++i)
    {
        for(int j = 0; j < i; ++j)
        {
            std::swap(elem[i][j], elem[j][i]);
        }
    }
}

void Mat4x4::visualize(Vec3fp position, const X_RenderContext& renderContext) const
{
    const X_Palette* p = x_palette_get_quake_palette();
    X_Color color[] = { p->brightRed, p->lightGreen, p->lightBlue };
    
    for(int i = 0; i < 3; ++i)
    {
        Vec4 v = getColumn(i);
        
        Vec3fp end = v.toVec3();
        end = end * 50;
        end += position;
        
        Ray3 r(position, end);
        r.render(renderContext, color[i]);
    }
}

void Mat4x4::extractEulerAngles(fp& xDest, fp& yDest) const
{
    Mat4x4 mat = *this;
    mat.dropTranslation();
    mat.transpose3x3();

    Vec3fp v(0, 0, fp::fromInt(1));
    Vec3fp transformed = mat.transform(v);

    Vec3fp up, forward, right;
    mat.extractViewVectors(forward, right, up);

    // I found these formulas experimentally...

    if(up.y < 0)
    {
        xDest = x_asin(transformed.y);
        yDest = x_atan2(transformed.z, transformed.x) - fp(X_ANG_90);
    }
    else
    {
        xDest = -x_asin(transformed.y) - fp(X_ANG_180);
        yDest = x_atan2(transformed.z, transformed.x) + fp(X_ANG_90);
    }

    adjustAngle(xDest);
    adjustAngle(yDest);
}



