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
#include "memory/OldLink.hpp"
#include "render/Texture.hpp"

struct X_BspNode;
struct X_BspClipNode;
struct BspSurface;
struct X_RenderContext;
struct BspPlane;
struct BspVertex;
struct BspEdge;

struct BspModel
{
    BoundBox boundBox;
    X_BspNode* rootBspNode;

    X_BspClipNode* clipNodes;
    int clipNodeRoots[3];

    BspPlane* planes;
    BspVertex* vertices;
    BspEdge* edges;
    int* surfaceEdgeIds;
    
    int totalBspLeaves;
    
    BspSurface* faces;
    int totalFaces;
    
    Vec3fp center;
    
    unsigned int flags;
    
    OldLink objectsOnModelHead;
    OldLink objectsOnModelTail;
};

