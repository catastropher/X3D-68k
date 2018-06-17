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

static bool is_leaf_node(int clipNodeId)
{
    return clipNodeId < 0;
}

X_BspLeafContents get_leaf_contents(int clipNodeId)
{
    return (X_BspLeafContents)clipNodeId;
}

static inline X_BspClipNode* x_bsplevel_get_clip_node(X_BspLevel* level, int clipNodeId)
{
    return level->clipNodes + clipNodeId;
}

static bool both_points_on_front_side(x_fp16x16 startDist, x_fp16x16 endDist)
{
    return startDist >= 0 && endDist >= 0;
}

static bool both_points_on_back_side(x_fp16x16 startDist, x_fp16x16 endDist)
{
    return startDist < 0 && endDist < 0;
}

static inline x_fp16x16 calculate_intersection_t(x_fp16x16 startDist, x_fp16x16 endDist)
{
    const x_fp16x16 DISTANCE_EPSILON = x_fp16x16_from_float(0.03125);
    x_fp16x16 top;
    if(startDist < 0)
        top = startDist + DISTANCE_EPSILON;
    else
        top = startDist - DISTANCE_EPSILON;
    
    x_fp16x16 t = x_fp16x16_div(top, startDist - endDist);
    
    return x_fp16x16_clamp(t, 0, X_FP16x16_ONE);
}

bool visit_node(X_RayTracer* trace, int clipNodeId, Vec3* start, x_fp16x16 startT, Vec3* end, x_fp16x16 endT);

static X_BspLeafContents get_clip_node_contents(X_BspLevel* level, int clipNodeId, Vec3* v)
{
    while(!is_leaf_node(clipNodeId))
    {
        X_BspClipNode* clipNode = x_bsplevel_get_clip_node(level, clipNodeId);
        X_Plane* plane = &level->planes[clipNode->planeId].plane;
        
        if(x_plane_point_distance(plane, v) >= 0)
            clipNodeId = clipNode->frontChild;
        else
            clipNodeId = clipNode->backChild;
    }
    
    return get_leaf_contents(clipNodeId);
}

static inline bool explore_both_sides_of_node(X_RayTracer* trace,
                                               X_BspClipNode* node,
                                               Vec3* start,
                                               x_fp16x16 startT,
                                               Vec3* end,
                                               x_fp16x16 endT,
                                               X_Plane* plane,
                                               x_fp16x16 intersectionT,
                                               x_fp16x16 startDist
                                              )
{
    X_Ray3_fp16x16 ray = x_ray3_make(*start, *end);
 
    Vec3 intersection;
    x_ray3_lerp(&ray, intersectionT, &intersection);
    
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
    
    if(!visit_node(trace, startChildNode, start, startT, &intersection, intersectionT))
        return 0;
    
    bool intersectionInSolidLeaf = get_clip_node_contents(trace->level, endChildNode, &intersection) == X_BSPLEAF_SOLID;
    if(!intersectionInSolidLeaf)
        return visit_node(trace, endChildNode, &intersection, intersectionT, end, endT);
    
    trace->collisionPlane = *plane;
    
    if(startDist < 0)
    {
        // Flip the direction of the plane because we're colliding on the wrong side
        x_plane_flip_direction(&trace->collisionPlane);
    }
    
    trace->collisionPoint = intersection;
    trace->collisionFrac = intersectionT;
    
    return 0;
}

bool visit_node(X_RayTracer* trace, int clipNodeId, Vec3* start, x_fp16x16 startT, Vec3* end, x_fp16x16 endT)
{
    if(is_leaf_node(clipNodeId))
        return 1;
    
    X_BspClipNode* node = x_bsplevel_get_clip_node(trace->level, clipNodeId);
    X_Plane* plane = &trace->level->planes[node->planeId].plane;
    
    x_fp16x16 startDist = x_plane_point_distance(plane, start);
    x_fp16x16 endDist = x_plane_point_distance(plane, end);
    
    if(both_points_on_front_side(startDist, endDist))
        return visit_node(trace, node->frontChild, start, startT, end, endT);
    else if(both_points_on_back_side(startDist, endDist))
        return visit_node(trace, node->backChild, start, startT, end, endT);
    
    // The ray spans the split plane, so we need to explore both sides
    x_fp16x16 intersectionT = calculate_intersection_t(startDist, endDist);
    
    return explore_both_sides_of_node(trace, node, start, startT, end, endT, plane, intersectionT, startDist);
}

void x_raytracer_init(X_RayTracer* trace, X_BspLevel* level, X_BspModel* model, Vec3* start, Vec3* end, X_BoundBox* boundBox)
{
    trace->modelOrigin = &model->origin;
    trace->level = level;
    trace->ray.v[0] = *start;
    trace->ray.v[1] = *end;
    trace->rootClipNode = model->clipNodeRoots[0];
}

static bool trace_model(X_RayTracer* trace, X_BspModel* model)
{
    Vec3 start = x_vec3_sub(trace->ray.v + 0, &model->origin);
    Vec3 end = x_vec3_sub(trace->ray.v + 1, &model->origin);
    
    bool success = !visit_node(trace, model->clipNodeRoots[0], &start, 0, &end, X_FP16x16_ONE);
    
    // Move the plane relative to the origin of the object
    trace->collisionPoint = trace->collisionPoint + model->origin;

    Vec3fp temp = MakeVec3fp(trace->collisionPoint);

    trace->collisionPlane.d = -trace->collisionPlane.normal.dot(temp);
    
    return success;
}

bool x_raytracer_trace(X_RayTracer* trace)
{
    X_RayTracer best = *trace;
    
    best.collisionFrac = X_FP16x16_ONE;
    
    // FIXME: this is not a good way to handle this
    bool success = 0;
    for(int i = 0; i < trace->level->totalModels; ++i)
    {
        if(trace_model(trace, trace->level->models + i))
        {
            success = 1;
            
            if(trace->collisionFrac < best.collisionFrac)
            {
                best = *trace;
                best.hitModel = trace->level->models + i;
            }
        }
    }
    
    *trace = best;
    
    return success;
}

