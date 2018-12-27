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

#include "math/X_fix.h"
#include "level/X_BspLevel.h"
#include "geo/X_Ray3.h"

struct RayPoint
{
    RayPoint() { }
    RayPoint(Vec3fp point_, fp t_) : point(point_), t(t_) { }

    Vec3fp point;
    fp t;
};

template<typename NodeId>
struct RayCollision
{
    bool isCloserThan(const RayCollision& collision) const
    {
        return location.t < collision.location.t;
    }

    RayPoint location;
    Plane plane;
    BspModel* hitModel;
    NodeId hitNode;
};

template<typename IdType, typename NodeType>
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
    bool traceModel(BspModel& model);

    RayCollision<IdType>& getCollision()
    {
        return collision;
    }

private:
    NodeType getNodeFromId(IdType id);
    X_BspLeafContents getNodeContents(IdType id, Vec3fp& v);
    bool exploreBothSidesOfNode(IdType nodeId, RayPoint& start, RayPoint& end, fp intersectionT, fp startDist);
    bool visitNode(IdType nodeId, RayPoint& start, RayPoint& end);
    X_BspLeafContents getLeafContents(IdType type);

    static bool nodeIsLeaf(IdType id);

    Plane& getNodePlane(NodeType node);

    IdType getRootNode(BspModel& model);

    Ray3 ray;
    BspLevel* level;
    RayCollision<IdType> collision;
    int collisionHullId;
    BspModel* currentModel;
};

