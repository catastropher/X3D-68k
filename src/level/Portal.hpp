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

#include "geo/BoundRect.hpp"
#include "geo/Plane.hpp"
#include "geo/Polygon3.hpp"
#include "math/Mat4x4.hpp"
#include "memory/BitSet.hpp"
#include "level/BspLevel.hpp"

struct X_AE_Surface;

enum PortalFlags
{
    PORTAL_ENABLED = 1,
    PORTAL_DRAW_OUTLINE = 2
};

struct Portal
{
    void linkTo(Portal* portal);

    Vec3fp transformPointToOtherSide(Vec3fp point);

    static void linkMutual(Portal* a, Portal* b)
    {
        if(a)
        {
            a->linkTo(b);
        }

        if(b)
        {
            b->linkTo(a);
        }
    }

    void updatePoly();

    void enableOutline(X_Color color)
    {
        outlineColor = color;
        flags.set(PORTAL_DRAW_OUTLINE);
    }

    bool pointInPortal(Vec3fp& point) const;

    bool pointInBox(const Vec3fp& point);

    Vec2fp projectPointOntoSurface(Vec3fp& point) const;

    Vec3fp& outlinePointWithLargestProjection(const Vec3fp& axis);

    Polygon3 poly;
    Vec3fp center;
    Plane plane;
    Mat4x4 orientation;
    Mat4x4 transformToOtherSide;
    X_AE_Surface* aeSurface;
    Portal* otherSide;
    EnumBitSet<PortalFlags> flags;
    X_Color outlineColor;

    fp transformAngleX;
    fp transformAngleY;

    BoundRect surfaceBoundRect;

    BspModel bridgeModel;
    X_BspPlane bridgePlanes[6];
    X_BspClipNode bridgeClipNodes[6];


    Portal* next;

private:
    void calculateSurfaceBoundRect();
};

