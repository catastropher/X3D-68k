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

#include "X_Camera.hpp"

void Camera::setView(const Vec3fp& newPosition, const Quaternion& newOrientation)
{
    position = newPosition;

    Mat4x4 rotation;
    newOrientation.toMat4x4(rotation);

    Mat4x4 translation;
    translation.loadTranslation(-newPosition);

    viewMatrix = translation * rotation;
}

void Camera::setView(const Vec3fp& newPosition, const Mat4x4& newViewMatrix)
{
    position = newPosition;
    viewMatrix = newViewMatrix;
}

void Camera::extractViewVectors(Vec3fp& forwardDest, Vec3fp& rightDest, Vec3fp& upDest) const
{
    viewMatrix.extractViewVectors(forwardDest, rightDest, upDest);
}

