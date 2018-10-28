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

namespace X3D
{
    void Mat4x4::loadIdentity()
    {
        static const Mat4x4 identity =
        {
            {
                { fp::fromInt(1), 0, 0, 0 },
                { 0, fp::fromInt(1), 0, 0 },
                { 0, 0, fp::fromInt(1), 0 },
                { 0, 0, 0, fp::fromInt(1) }
            }
        };
        
        *this = identity;
    }

    void Mat4x4::loadXRotation(Angle angle) 
    {
        fp cosAngle = angle.cos();
        fp sinAngle = angle.sin();
        
        Mat4x4 xRotation = {
            {
                { fp::fromInt(1), 0, 0, 0 },
                { 0, cosAngle, -sinAngle, 0 },
                { 0, sinAngle, cosAngle, 0 },
                { 0, 0, 0, fp::fromInt(1) }
            }
        };
        
        *this = xRotation;
    }

    void Mat4x4::loadYRotation(Angle angle)
    {
        fp cosAngle = angle.cos();
        fp sinAngle = angle.sin();
        
        Mat4x4 yRotation = {
            {
                { cosAngle, 0, sinAngle, 0 },
                { 0, fp::fromInt(1), 0, 0 },
                { -sinAngle, 0, cosAngle, 0 },
                { 0, 0, 0, fp::fromInt(1) }
            }
        };
        
        *this = yRotation;
    }

    void Mat4x4::loadZRotation(Angle angle)
    {
        fp cosAngle = angle.cos();
        fp sinAngle = angle.sin();
        
        Mat4x4 zRotation = {
            {
                { cosAngle, -sinAngle, 0, 0 },
                { sinAngle, cosAngle, 0, 0 },
                { 0, 0, fp::fromInt(1), 0 },
                { 0, 0, 0, fp::fromInt(1) }
            }
        };
        
        *this = zRotation;
    }

    void Mat4x4::loadTranslation(const Vec3& translation)
    {
        Mat4x4 translationMatrix = 
        {
            {
                { fp::fromInt(1), 0, 0, translation.x },
                { 0, fp::fromInt(1), 0, translation.y },
                { 0, 0, fp::fromInt(1), translation.z },
                { 0, 0, 0, fp::fromInt(1) }
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

    Vec3 Mat4x4::transform(const Vec3& src) const
    {
        Vec4 v = Vec4(src.x, src.y, src.z, fp::fromInt(1));
        Vec4 transformed = transform(v);

        return transformed.toVec3();
    }

    Vec3 Mat4x4::transformNormal(const Vec3& normal) const
    {
        return Vec3(
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

    void Mat4x4::extractViewVectors(Vec3& forwardDest, Vec3& rightDest, Vec3& upDest) const
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

    void Mat4x4::extractEulerAngles(Angle& xDest, Angle& yDest) const
    {
        Mat4x4 mat = *this;
        mat.dropTranslation();
        mat.transpose3x3();

        Vec3 v(0, 0, fp::fromInt(1));
        Vec3 transformed = mat.transform(v);

        Vec3 up, forward, right;
        mat.extractViewVectors(forward, right, up);

        // I found these formulas experimentally...

        if(up.y < 0)
        {
            xDest = Angle::asin(transformed.y);
            yDest = Angle::atan2(transformed.z, transformed.x) - Angle::fromDegrees(90);
        }
        else
        {
            xDest = -Angle::asin(transformed.y) - Angle::fromDegrees(180);
            yDest = Angle::atan2(transformed.z, transformed.x) + Angle::fromDegrees(90);
        }
    }
}

