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

#include "math/X_Mat4x4.h"
#include "geo/X_Vec3.h"
#include "X_Viewport.h"
#include "math/X_Quaternion.h"

class Camera
{
public:
    Mat4x4& getViewMatrix()
    {
        return viewMatrix;
    }

    Vec3fp& getPosition()
    {
        return position;
    }

    void setView(const Vec3fp& newPosition, const Quaternion& newOrientation);
    void setView(const Vec3fp& newPosition, const Mat4x4& newViewMatrix);

    void extractViewVectors(Vec3fp& forwardDest, Vec3fp& rightDest, Vec3fp& upDest) const;

private:
    Vec3fp position;
    Mat4x4 viewMatrix;
};

