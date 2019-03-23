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

#include "entity/component/BrushModelComponent.hpp"
#include "math/FixedPoint.hpp"
#include "level/BspLevel.hpp"
#include "geo/Ray3.hpp"

class Entity;

struct RayPoint
{
    RayPoint() { }
    RayPoint(Vec3fp point_, fp t_) : point(point_), t(t_) { }

    Vec3fp point;
    fp t;
};

struct RayCollision
{
    RayCollision()
        : entity(nullptr)
    {

    }

    bool isCloserThan(const RayCollision& collision) const
    {
        return location.t < collision.location.t;
    }

    RayPoint location;
    Plane plane;
    BspModel* hitModel;
    int hitNode;
    Entity* entity;
};

class BspRayTracer
{
public:
    BspRayTracer(const Ray3& ray_, BspLevel* level_, int collisionHullId_)
        : ray(ray_),
        level(level_),
        collisionHullId(collisionHullId_)
    {
        collision.location.t = maxValue<fp>();
    }

    bool trace();
    bool traceModel(Entity* entityWithModel);

    RayCollision& getCollision()
    {
        return collision;
    }

private:
    Plane& getNodePlane(X_BspClipNode* node)
    {
        return currentModel->planes[node->planeId].plane;
    }
    
    static bool nodeIsLeaf(int clipNodeId)
    {
        return clipNodeId < 0;
    }
    
    X_BspClipNode* getNodeFromId(int id)
    {
        return currentModel->clipNodes + id;
    }
    
    X_BspLeafContents getLeafContents(int id)
    {
        return (X_BspLeafContents)id;
    }
    
    int getRootNode(BspModel& model)
    {
        return model.clipNodeRoots[collisionHullId];
    }
    
    bool exploreBothSidesOfNode(int nodeId, RayPoint& start, RayPoint& end, fp intersectionT, fp startDist);
    bool visitNode(int nodeId, RayPoint& start, RayPoint& end);
    X_BspLeafContents getNodeContents(int id, Vec3fp& v);

    Ray3 ray;
    BspLevel* level;
    RayCollision collision;
    int collisionHullId;
    BspModel* currentModel;
    Entity* currentModelOwner;
};

