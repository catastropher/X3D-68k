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

#include "Vec3.hpp"
#include "Vec4.hpp"

namespace X3D
{
    struct Mat4x4
    {
        void dropTranslation()
        {
            elem[0][3] = 0;
            elem[1][3] = 0;
            elem[2][3] = 0;
        }

        void loadIdentity();
        void loadXRotation(fp angle);
        void loadYRotation(fp angle);
        void loadZRotation(fp angle);
        void loadTranslation(const Vec3& translation);

        Vec4 getColumn(int col) const;
        Vec4 getRow(int row) const;

        void setColumn(int col, const Vec4& v);
        void setRow(int col, const Vec4& v);

        Vec4 transform(const Vec4& src) const;
        Vec3 transform(const Vec3& src) const;
        Vec3 transformNormal(const Vec3& normal) const;

        void print() const;

        void extractViewVectors(Vec3& forwardDest, Vec3& rightDest, Vec3& upDest) const;
        void extractEulerAngles(fp& xDest, fp& yDest) const;

        void invertDiagonal(Mat4x4 dest) const;
        void transpose3x3();

        Mat4x4 operator*(const Mat4x4& mat) const;

        fp elem[4][4];
    };
}

