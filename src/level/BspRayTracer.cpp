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

#include "BspRayTracer.hpp"
#include "entity/component/BrushModelComponent.hpp"

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

X_BspLeafContents BspRayTracer::getNodeContents(int id, Vec3fp& v)
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

bool BspRayTracer::exploreBothSidesOfNode(int nodeId, RayPoint& start, RayPoint& end, fp intersectionT, fp startDist)
{
    X_BspClipNode* node = getNodeFromId(nodeId);

    Ray3 ray(start.point, end.point);
 
    RayPoint intersection(ray.lerp(intersectionT), intersectionT);
    
    int startChildNode;
    int endChildNode;
    
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
    collision.hitNode = nodeId;
    collision.entity = currentModelOwner;

    // Move the plane relative to the origin of the object
    collision.location.point = collision.location.point + currentModel->center;
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

bool BspRayTracer::visitNode(int nodeId, RayPoint& start, RayPoint& end)
{
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
    
    return exploreBothSidesOfNode(nodeId, start, end, intersectionT, startDist);
}

bool BspRayTracer::traceModel(BrushModelComponent& brushModel)
{
// FIXME: 2-20-2019
#if false
    currentModel = brushModel.model;
    currentModelOwner = brushModel.owner;
#endif

    RayPoint start(ray.v[0] - brushModel.model->center, 0);
    RayPoint end(ray.v[1] - brushModel.model->center, fp::fromInt(1));

    bool hitSomething = !visitNode(getRootNode(*brushModel.model), start, end);
    
    return hitSomething;
}

bool BspRayTracer::trace()
{
    collision.location.t = maxValue<fp>();

// FIXME: 2-20-2019
#if false
    // FIXME: this could very easily be optimized
    
    auto brushModels = BrushModelComponent::getAll();
    
    bool hitSomething = false;
    
    for(auto& brushModel : brushModels)
    {
        if(brushModel.model != nullptr)
        {
            hitSomething |= traceModel(brushModel);
        }
    }
    
    return hitSomething;
#endif

    return false;
}


