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

static _Bool is_leaf_node(int clipNodeId)
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

static _Bool both_points_on_front_side(x_fp16x16 startDist, x_fp16x16 endDist)
{
    return startDist >= 0 && endDist >= 0;
}

static _Bool both_points_on_back_side(x_fp16x16 startDist, x_fp16x16 endDist)
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
    
    x_fp16x16 t = top / (startDist - endDist);
    return x_fp16x16_clamp(t, 0, X_FP16x16_ONE);
}

_Bool visit_node(X_RayTracer* trace, int clipNodeId, X_Vec3_fp16x16* start, x_fp16x16 startT, X_Vec3_fp16x16* end, x_fp16x16 endT);

static X_BspLeafContents get_clip_node_contents(X_BspLevel* level, int clipNodeId, X_Vec3_fp16x16* v)
{
    while(!is_leaf_node(clipNodeId))
    {
        X_BspClipNode* clipNode = x_bsplevel_get_clip_node(level, clipNodeId);
        X_Plane* plane = &level->planes[clipNode->planeId].plane;
        
        if(x_plane_point_distance_fp16x16(plane, v) >= 0)
            clipNodeId = clipNode->frontChild;
        else
            clipNodeId = clipNode->backChild;
    }
    
    return get_leaf_contents(clipNodeId);
}

static inline _Bool explore_both_sides_of_node(X_RayTracer* trace,
                                               X_BspClipNode* node,
                                               X_Vec3_fp16x16* start,
                                               x_fp16x16 startT,
                                               X_Vec3_fp16x16* end,
                                               x_fp16x16 endT,
                                               X_Plane* plane,
                                               x_fp16x16 intersectionT,
                                               x_fp16x16 startDist
                                              )
{
    X_Ray3_fp16x16 ray = x_ray3_make(*start, *end);
 
    X_Vec3_fp16x16 intersection;
    x_ray3_fp16x16_lerp(&ray, intersectionT, &intersection);
    
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
    
    _Bool intersectionInSolidLeaf = get_clip_node_contents(trace->level, endChildNode, &intersection) == X_BSPLEAF_SOLID;
    if(!intersectionInSolidLeaf)
        return visit_node(trace, endChildNode, &intersection, intersectionT, end, endT);
    
    printf("Found impact!\n");
    return 0;
}

_Bool visit_node(X_RayTracer* trace, int clipNodeId, X_Vec3_fp16x16* start, x_fp16x16 startT, X_Vec3_fp16x16* end, x_fp16x16 endT)
{
    if(is_leaf_node(clipNodeId))
        return 1;
    
    X_BspClipNode* node = x_bsplevel_get_clip_node(trace->level, clipNodeId);
    X_Plane* plane = &trace->level->planes[node->planeId].plane;
    
    x_fp16x16 startDist = x_plane_point_distance_fp16x16(plane, start);
    x_fp16x16 endDist = x_plane_point_distance_fp16x16(plane, end);
    
    if(both_points_on_front_side(startDist, endDist))
        return visit_node(trace, node->frontChild, start, startT, end, endT);
    else if(both_points_on_back_side(startDist, endDist))
        return visit_node(trace, node->backChild, start, startT, end, endT);
    
    // The ray spans the split plane, so we need to explore both sides
    x_fp16x16 intersectionT = calculate_intersection_t(startDist, endDist);
    
    return explore_both_sides_of_node(trace, node, start, startT, end, endT, plane, intersectionT, startDist);
}

void x_raytracer_init(X_RayTracer* trace, X_BspLevel* level, X_Vec3_fp16x16* start, X_Vec3_fp16x16* end, X_BspBoundBox* boundBox)
{
    trace->level = level;
    trace->ray.v[0] = *start;
    trace->ray.v[1] = *end;
    trace->rootClipNode = 0;
}

_Bool x_raytracer_trace(X_RayTracer* trace)
{
    return !visit_node(trace, trace->rootClipNode, trace->ray.v + 0, 0, trace->ray.v + 1, X_FP16x16_ONE);
}

