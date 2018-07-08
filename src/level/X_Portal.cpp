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

#include "X_Portal.hpp"

void Portal::linkTo(Portal* otherSide_)
{
    otherSide = otherSide_;

    if(otherSide == nullptr)
    {
        return;
    }

    Mat4x4 rotate180DegreesAroundY;
    rotate180DegreesAroundY.loadYRotation(0);

    Mat4x4 transpose = orientation;
    transpose.transpose3x3();

    Mat4x4 translation;
    translation.loadTranslation(-center);

    Mat4x4 othersideFlipped = rotate180DegreesAroundY * otherSide->orientation;

    transformToOtherSide = othersideFlipped * transpose * translation;


    // Mat4x4 otherSideTranslation;
    // otherSideTranslation.loadTranslation(-otherSide->center);

    // Mat4x4 otherSideTranspose = otherSide->orientation;
    // otherSideTranspose.transpose3x3();

    // Mat4x4 inverseOtherSideObjecToWorld = otherSideTranslation * otherSideTranspose;

    // Mat4x4 translation;
    // translation.loadTranslation(center);

    // Mat4x4 objectToWorld = orientation * translation;

    // Mat4x4 rotate180DegreesAroundY;
    // rotate180DegreesAroundY.loadYRotation(0);

    // transformToOtherSide = objectToWorld * rotate180DegreesAroundY * inverseOtherSideObjecToWorld;

    // transformToOtherSide.print();
}

Vec3fp Portal::transformPointToOtherSide(Vec3fp point)
{
    Vec3fp diff = point - center;

    Mat4x4 mat = transformToOtherSide;
    mat.dropTranslation();
    mat.transpose3x3();

    return mat.transform(diff) + otherSide->center;
}

