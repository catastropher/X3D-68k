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

#include "geo/BoundBox.hpp"
#include "render/Texture.hpp"
#include "PotentiallyVisibleSet.hpp"

struct X_RenderContext;
struct BspModel;
struct BspPlane;
struct BspSurface;
struct X_BspLeaf;

typedef enum X_BspLeafContents
{
    X_BSPLEAF_NODE = 0,
    X_BSPLEAF_REGULAR = -1,
    X_BSPLEAF_SOLID = -2,
    X_BSPLEAF_WATER = -3,
    X_BSPLEAF_SLIME = -4,
    X_BSPLEAF_LAVA = -5,
    X_BSPLEAF_SKY = -6
} X_BspLeafContents;

typedef struct X_BspNode
{
    bool isLeaf() const
    {
        return contents < 0;
    }

    X_BspLeaf& getLeaf()
    {
        return *(X_BspLeaf*)this;
    }

    bool isVisibleThisFrame(int currentFrame) const
    {
        return lastVisibleFrame == currentFrame;
    }

    void markAncestorsAsVisible(int currentFrame);

    // Common with X_BspLeaf - DO NOT REORDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    BoundBox nodeBoundBox;
    BoundBox geoBoundBox;
    X_BspNode* parent;
    
    // Unique elements for node
    BspPlane* plane;
    
    X_BspNode* frontChild;
    X_BspNode* backChild;
    
    BspSurface* firstSurface;
    int totalSurfaces;
private:
} X_BspNode;

typedef struct X_BspLeaf
{
    bool isOutsideLevel() const
    {
        return contents == X_BSPLEAF_SOLID;
    }

    void markSurfacesAsVisible(int currentFrame, int bspKey_);

    // Common with X_BspNode - DO NOT REOREDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    BoundBox nodeBoundBox;
    BoundBox geoBoundBox;
    struct X_BspNode* parent;
    
    // Unique elements for leaf
    BspSurface** firstMarkSurface;
    int totalMarkSurfaces;
    int bspKey;
    
    CompressedLeafVisibleSet pvsFromLeaf;
} X_BspLeaf;

