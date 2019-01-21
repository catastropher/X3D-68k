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

#include "geo/X_BoundBox.h"
#include "memory/X_Link.h"
#include "render/X_Texture.h"

struct X_BspNode;
struct X_BspClipNode;
struct X_BspSurface;
struct X_RenderContext;
struct X_BspPlane;
struct X_BspVertex;
struct X_BspEdge;

struct BspModel
{
    BoundBox boundBox;
    X_BspNode* rootBspNode;

    X_BspClipNode* clipNodes;
    int clipNodeRoots[3];

    X_BspPlane* planes;
    X_BspVertex* vertices;
    X_BspEdge* edges;
    int* surfaceEdgeIds;
    
    int totalBspLeaves;
    
    X_BspSurface* faces;
    int totalFaces;
    
    Vec3fp center;
    
    unsigned int flags;
    
    X_Link objectsOnModelHead;
    X_Link objectsOnModelTail;
};

