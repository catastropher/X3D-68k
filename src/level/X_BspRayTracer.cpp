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

#include "X_BspRayTracer.hpp"

template class BspRayTracer<int, X_BspClipNode*>;
template class BspRayTracer<X_BspNode*, X_BspNode*>;

static fp calculateIntersectionT(fp startDist, fp endDist)
{
    const fp DISTANCE_EPSILON = fp::fromFloat(0.03125);
    fp top;

    if(startDist < 0)
        top = startDist + DISTANCE_EPSILON;
    else
        top = startDist - DISTANCE_EPSILON;
    
    fp t = top / (startDist - endDist);
    
    return clamp(t, fp::fromInt(0), fp::fromInt(1));
}

template<typename IdType, typename NodeType>
X_BspLeafContents BspRayTracer<IdType, NodeType>::getNodeContents(IdType id, Vec3fp& v)
{
    while(!nodeIsLeaf(id))
    {
        auto node = getNodeFromId(id);
        Plane& plane = getNodePlane(node);

        if(plane.pointOnNormalFacingSide(v))
        {
            id = node->frontChild;
        }
        else
        {
            id = node->backChild;
        }
    }
    
    return getLeafContents(id);
}

template<typename IdType, typename NodeType>
bool BspRayTracer<IdType, NodeType>::exploreBothSidesOfNode(NodeType node, RayPoint& start, RayPoint& end, fp intersectionT, fp startDist)
{
    Ray3 ray(start.point, end.point);
 
    RayPoint intersection(ray.lerp(intersectionT), intersectionT);
    
    IdType startChildNode;
    IdType endChildNode;
    
    if(startDist >= 0)
    {
        startChildNode = node->frontChild;
        endChildNode = node->backChild;
    }
    else
    {
        startChildNode = node->backChild;
        endChildNode = node->frontChild;
    }
    
    if(!visitNode(startChildNode, start, intersection))
        return 0;
    
    bool intersectionInSolidLeaf = getNodeContents(endChildNode, intersection.point) == X_BSPLEAF_SOLID;
    if(!intersectionInSolidLeaf)
        return visitNode(endChildNode, intersection, end);

    if(intersection.t > collision.location.t)
    {
        return false;
    }
    
    collision.plane = getNodePlane(node);
    
    if(startDist < 0)
    {
        // Flip the direction of the plane because we're colliding on the wrong side
        collision.plane.flip();
    }
    
    collision.location = intersection;
    collision.hitModel = currentModel;

    // Move the plane relative to the origin of the object
    collision.location.point = collision.location.point + MakeVec3fp(currentModel->origin);
    collision.plane.d = -collision.plane.normal.dot(collision.location.point);
    
    return 0;
}

static bool both_points_on_front_side(fp startDist, fp endDist)
{
    return startDist >= 0 && endDist >= 0;
}

static bool both_points_on_back_side(fp startDist, fp endDist)
{
    return startDist < 0 && endDist < 0;
}

template<typename IdType, typename NodeType>
bool BspRayTracer<IdType, NodeType>::visitNode(IdType nodeId, RayPoint& start, RayPoint& end)
{
    printf("    Visit node: %lld\n", (long long)nodeId);

    if(nodeIsLeaf(nodeId))
    {
        return true;
    }
    
    auto node = getNodeFromId(nodeId);
    Plane& plane = getNodePlane(node);

    fp startDist = plane.distanceTo(start.point);
    fp endDist = plane.distanceTo(end.point);
    
    if(both_points_on_front_side(startDist, endDist))
    {
        return visitNode(node->frontChild, start, end);
    }
    else if(both_points_on_back_side(startDist, endDist))
    {
        return visitNode(node->backChild, start, end);
    }
    
    // The ray spans the split plane, so we need to explore both sides
    fp intersectionT = calculateIntersectionT(startDist, endDist).toFp16x16();
    
    return exploreBothSidesOfNode(node, start, end, intersectionT, startDist);
}

template<typename IdType, typename NodeType>
bool BspRayTracer<IdType, NodeType>::traceModel(X_BspModel& model)
{
    printf("Trace model %lld\n", (long long)&model);
    currentModel = &model;

    RayPoint start(ray.v[0] - MakeVec3fp(model.origin), 0);
    RayPoint end(ray.v[1] - MakeVec3fp(model.origin), fp::fromInt(1));

    bool hitSomething = !visitNode(getRootNode(model), start, end);        
    
    return hitSomething;
}

template<typename IdType, typename NodeType>
bool BspRayTracer<IdType, NodeType>::trace()
{
    collision.location.t = maxValue<fp>();

    // FIXME: this is not a good way to handle this
    bool hitSomething = false;
    for(int i = 0; i < level->totalModels; ++i)
    {
        hitSomething |= traceModel(level->models[i]);
    }
    
    return hitSomething;
}

// Specializations for clip nodes

template<>
Plane& BspRayTracer<int, X_BspClipNode*>::getNodePlane(X_BspClipNode* node)
{
    return currentModel->planes[node->planeId].plane;
}

template<>
bool BspRayTracer<int, X_BspClipNode*>::nodeIsLeaf(int clipNodeId)
{
    return clipNodeId < 0;
}

template<>
X_BspClipNode* BspRayTracer<int, X_BspClipNode*>::getNodeFromId(int id)
{
    return currentModel->clipNodes + id;
}

template<>
X_BspLeafContents BspRayTracer<int, X_BspClipNode*>::getLeafContents(int id)
{
    return (X_BspLeafContents)id;
}

template<>
int BspRayTracer<int, X_BspClipNode*>::getRootNode(X_BspModel& model)
{
    return model.clipNodeRoots[collisionHullId];
}

// Specializations for bsp nodes

template<>
Plane& BspRayTracer<X_BspNode*, X_BspNode*>::getNodePlane(X_BspNode* node)
{
    return node->plane->plane;
}

template<>
bool BspRayTracer<X_BspNode*, X_BspNode*>::nodeIsLeaf(X_BspNode* node)
{
    return node->isLeaf();
}

template<>
X_BspNode* BspRayTracer<X_BspNode*, X_BspNode*>::getNodeFromId(X_BspNode* node)
{
    return node;
}


template<>
X_BspLeafContents BspRayTracer<X_BspNode*, X_BspNode*>::getLeafContents(X_BspNode* id)
{
    return (X_BspLeafContents)id->contents;
}

template<>
X_BspNode* BspRayTracer<X_BspNode*, X_BspNode*>::getRootNode(X_BspModel& model)
{
    return model.rootBspNode;
}

