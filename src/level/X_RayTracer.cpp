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

#include "X_RayTracer.h"
#include "X_BspRayTracer.hpp"

template<typename T>
bool nodeIsLeaf(T id);

template<typename IdType, typename NodeType>
NodeType getNodeFromId(IdType idType, X_BspLevel* level);

template<typename IdType>
X_BspLeafContents getLeafContents(IdType type);

// Clipnodes

template<>
bool nodeIsLeaf(int clipNodeId)
{
    return clipNodeId < 0;
}

template<>
X_BspClipNode* getNodeFromId(int id, X_BspLevel* level)
{
    return level->clipNodes + id;
}

template<>
X_BspLeafContents getLeafContents(int id)
{
    return (X_BspLeafContents)id;
}

// BspNode

template<>
bool nodeIsLeaf(X_BspNode* node)
{
    return node->isLeaf();
}

template<>
X_BspNode* getNodeFromId(X_BspNode* id, X_BspLevel* level)
{
    return id;
}


template<>
X_BspLeafContents getLeafContents(X_BspNode* id)
{
    return (X_BspLeafContents)id->contents;
}

void x_raytracer_init(X_RayTracer* trace, X_BspLevel* level, X_BspModel* model, Vec3* start, Vec3* end, BoundBox* boundBox)
{
    Ray3 ray(MakeVec3fp(*start), MakeVec3fp(*end));

    BspRayTracer<int, X_BspClipNode*> bspRayTracer(ray, level, 0);

    trace->hitSomething = bspRayTracer.trace();

    auto& collision = bspRayTracer.getCollision();

    trace->hitModel = collision.hitModel;
    trace->collisionFrac = collision.location.t.toFp16x16();
    trace->collisionPlane = collision.plane;
    trace->collisionPoint = MakeVec3(collision.location.point);
}

bool x_raytracer_trace(X_RayTracer* trace)
{
    return trace->hitSomething;
}

