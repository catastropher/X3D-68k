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

struct X_RenderContext;

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
    void loadTranslation(const Vec3fp& translation);

    Vec4 getColumn(int col) const;
    Vec4 getRow(int row) const;

    void setColumn(int col, const Vec4& v);
    void setRow(int col, const Vec4& v);

    Vec4 transform(const Vec4& src) const;
    Vec3fp transform(const Vec3fp& src) const;
    Vec3fp transformNormal(const Vec3fp& normal) const;

    void print() const;

    void extractViewVectors(Vec3fp& forwardDest, Vec3fp& rightDest, Vec3fp& upDest) const;
    void invertDiagonal(Mat4x4 dest) const;
    void transpose3x3();
    void visualize(Vec3fp position, const X_RenderContext& renderContext) const;

    Mat4x4 operator*(const Mat4x4& mat) const;

    fp elem[4][4];
};
