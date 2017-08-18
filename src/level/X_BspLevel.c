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

#include "geo/X_Ray3.h"
#include "render/X_activeedge.h"
#include "render/X_RenderContext.h"
#include "render/X_Renderer.h"
#include "X_BspLevel.h"
#include "X_BspLevelLoader.h"
#include "error/X_error.h"

X_BoundBoxPlaneFlags x_bspboundbox_determine_plane_clip_flags(X_BspBoundBox* box, X_Plane* plane)
{
    int px = (plane->normal.x > 0 ? 1 : 0);
    int py = (plane->normal.y > 0 ? 1 : 0);
    int pz = (plane->normal.z > 0 ? 1 : 0);
    
    X_Vec3 furthestPointAlongNormal = x_vec3_make(box->v[px].x, box->v[py].y, box->v[pz].z);
    if(!x_plane_point_is_on_normal_facing_side(plane, &furthestPointAlongNormal))
        return X_BOUNDBOX_OUTSIDE_PLANE;
    
    X_Vec3 closestPointAlongNormal = x_vec3_make(box->v[px ^ 1].x, box->v[py ^ 1].y, box->v[pz ^ 1].z);
    if(x_plane_point_is_on_normal_facing_side(plane, &closestPointAlongNormal))
        return X_BOUNDBOX_INSIDE_PLANE;
    
    return X_BOUNDBOX_INTERSECT_PLANE;
}

_Bool x_boundbox_clip_against_frustum_plane(X_BspBoundBoxFrustumFlags flags, int planeId)
{
    return flags & (1 << planeId);
}

// Based on an algorithm described at http://www.txutxi.com/?p=584
X_BspBoundBoxFrustumFlags x_bspboundbox_determine_frustum_clip_flags(X_BspBoundBox* box, X_Frustum* frustum, X_BspBoundBoxFrustumFlags parentFlags)
{
    if(parentFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
        return X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM;
    
    X_BspBoundBoxFrustumFlags newFlags = 0;
    
    for(int i = 0; i < frustum->totalPlanes; ++i)
    {
        if(!x_boundbox_clip_against_frustum_plane(parentFlags, i))
            continue;
        
        X_BoundBoxPlaneFlags planeFlags = x_bspboundbox_determine_plane_clip_flags(box, frustum->planes + i);
        
        if(planeFlags == X_BOUNDBOX_OUTSIDE_PLANE)
            return X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM;
        
        if(planeFlags == X_BOUNDBOX_INTERSECT_PLANE)
            newFlags |= (1 << i);
    }
    
    return newFlags;
}

void x_bsplevel_render_wireframe(X_BspLevel* level, X_RenderContext* rcontext, X_Color color)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        X_BspEdge* edge = level->edges + i;
        
        X_Ray3 ray = x_ray3_make
        (
            x_vec3_fp16x16_to_vec3(&level->vertices[edge->v[0]].v),
            x_vec3_fp16x16_to_vec3(&level->vertices[edge->v[1]].v)
        );
        
        x_ray3d_render(&ray, rcontext, color);
    }
}

X_BspLeaf* x_bsplevel_find_leaf_point_is_in(X_BspLevel* level, X_Vec3* point)
{
    X_BspNode* node = x_bsplevel_get_root_node(level);
 
    do
    {
        node = x_plane_point_is_on_normal_facing_side(&node->plane->plane, point) ? node->frontChild : node->backChild;
    } while(!x_bspnode_is_leaf(node));
    
    return (X_BspLeaf*)node;
}

void x_bsplevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLeaf* leaf, unsigned char* decompressedPvsDest)
{
    // The PVS is compressed using zero run-length encoding
    int pos = 0;
    int pvsSize = x_bspfile_node_pvs_size(level);
    unsigned char* pvsData = leaf->compressedPvsData;
    
    _Bool outsideLevel = leaf == level->leaves + 0;
    
    // No visibility info (whoever compiled the map didn't run the vis tool)
    if(pvsData == NULL || outsideLevel)
    {
        // Mark all leaves as visible
        for(int i = 0; i < pvsSize; ++i)
            decompressedPvsDest[i] = 0xFF;
            
        return;
    }
    
    while(pos < pvsSize)
    {
        if(*pvsData == 0)
        {
            ++pvsData;
            int count = *pvsData++;
            
            for(int i = 0; i < count && pos < pvsSize; ++i)
                decompressedPvsDest[pos++] = 0;
        }
        else
        {
            decompressedPvsDest[pos++] = *pvsData++;
        }
    }
}

int x_bsplevel_count_visible_leaves(X_BspLevel* level, unsigned char* pvs)
{
    int count = 0;
    
    for(int i = 0; i < x_bsplevel_get_level_model(level)->totalBspLeaves; ++i)
    {
        if(pvs[i / 8] & (1 << (i % 8)))
            ++count;
    }
    
    return count;
}

void x_bsplevel_init_empty(X_BspLevel* level)
{    
    level->flags = 0;
}

static void x_bspnode_mark_all_parents_as_visible(X_BspNode* node, int currentFrame)
{
    do
    {
        // Don't bother walking all the way up the tree if we've already marked them as visible
        if(node->lastVisibleFrame == currentFrame)
            break;
        
        node->lastVisibleFrame = currentFrame;
        node = node->parent;
    } while(node != NULL);
}

void x_bsplevel_mark_visible_leaves_from_pvs(X_BspLevel* level, unsigned char* pvs, int currentFrame)
{
    int totalLeaves = x_bsplevel_get_level_model(level)->totalBspLeaves;
    
    for(int i = 0; i < totalLeaves; ++i)
    {
        _Bool leafVisible = pvs[i / 8] & (1 << (i & 7));
        X_BspNode* leafNode = (X_BspNode*)x_bsplevel_get_leaf(level, i + 1);    // PVS excludes leaf 0 so we start at leaf 1
        
        if(leafVisible)
            x_bspnode_mark_all_parents_as_visible(leafNode, currentFrame);
    }
}

static void x_bspleaf_mark_all_surfaces_in_leaf_as_visible(X_BspLeaf* leaf, int currentFrame)
{
    X_BspSurface** nextSurface = leaf->firstMarkSurface;
    
    for(int i = 0; i < leaf->totalMarkSurfaces; ++i)
    {
        X_BspSurface* surface = *nextSurface;        
        surface->lastVisibleFrame = currentFrame;
        ++nextSurface;
    }
}

static void x_bspnode_mark_surfaces_in_node_as_close_to_light(X_BspNode* node, const X_Light* light, int currentFrame)
{
    for(int i = 0; i < node->totalSurfaces; ++i)
    {
        X_BspSurface* surface = node->firstSurface + i;
        
        if(surface->lastLightUpdateFrame != currentFrame)
        {
            surface->lastLightUpdateFrame = currentFrame;
            surface->lightsTouchingSurface = 0;
        }
        
        surface->lightsTouchingSurface |= (1 << light->id);
    }
}

static void x_bspnode_mark_surfaces_light_is_close_to(X_BspNode* node, const X_Light* light, int currentFrame)
{
    if(x_bspnode_is_leaf(node))
        return;
    
    X_Plane* plane = &node->plane->plane;
    X_Vec3 lightPos = x_vec3_fp16x16_to_vec3(&light->position);
    int dist = x_fp16x16_to_int(x_plane_point_distance(plane, &lightPos));
    
    if(dist > light->intensity)
    {
        // Too far away from this node on the front side
        x_bspnode_mark_surfaces_light_is_close_to(node->frontChild, light, currentFrame);
        return;
    }
    
    if(dist < -light->intensity)
    {
        // Too far away from this node on the back side
        x_bspnode_mark_surfaces_light_is_close_to(node->backChild, light, currentFrame);
        return;
    }
    
    x_bspnode_mark_surfaces_in_node_as_close_to_light(node, light, currentFrame);
    x_bspnode_mark_surfaces_light_is_close_to(node->frontChild, light, currentFrame);
    x_bspnode_mark_surfaces_light_is_close_to(node->backChild, light, currentFrame);
}

void x_bsplevel_mark_surfaces_light_is_close_to(X_BspLevel* level, const X_Light* light, int currentFrame)
{
    x_bspnode_mark_surfaces_light_is_close_to(x_bsplevel_get_level_model(level)->rootBspNode, light, currentFrame);
}

void x_bsplevel_draw_edges_in_leaf(X_BspLevel* level, X_BspLeaf* leaf, X_RenderContext* renderContext, X_Color color)
{
    X_BspSurface** nextSurface = leaf->firstMarkSurface;
    
    for(int i = 0; i < leaf->totalMarkSurfaces; ++i)
    {
        X_BspSurface* surface = *nextSurface++;
        
        for(int j = 0; j < surface->totalEdges; ++j)
        {
            int edgeId = level->surfaceEdgeIds[surface->firstEdgeId + j];
            
            X_BspEdge* edge = level->edges + abs(edgeId);
            
            X_Ray3 ray = x_ray3_make
            (
                level->vertices[edge->v[0]].v,
                level->vertices[edge->v[1]].v
            );
            
            x_ray3d_render(&ray, renderContext, color);
        }
    }
}

static void x_bspnode_determine_children_sides_relative_to_camera(const X_BspNode* node, const X_Vec3* camPos, X_BspNode** frontSide, X_BspNode** backSide)
{
    _Bool onNormalSide = x_plane_point_is_on_normal_facing_side(&node->plane->plane, camPos);
    
    if(onNormalSide)
    {
        *frontSide = node->frontChild;
        *backSide = node->backChild;
    }
    else
    {
        *frontSide = node->backChild;
        *backSide = node->frontChild;
    }
}

static void x_bspnode_render_surfaces(X_BspNode* node, X_RenderContext* renderContext, X_BspBoundBoxFrustumFlags geoFlags)
{
    X_BspLevel* level = renderContext->level;
    
    for(int i = 0; i < node->totalSurfaces; ++i)
    {
        X_BspSurface* surface = node->firstSurface + i;
        
        if(!x_bspsurface_is_visible_this_frame(surface, renderContext->currentFrame))
            continue;
        
        _Bool onNormalSide = x_plane_point_is_on_normal_facing_side(&surface->plane->plane, &renderContext->camPos);
        _Bool planeFlipped = (surface->flags & X_BSPSURFACE_FLIPPED) != 0;
        
        if((!onNormalSide) ^ planeFlipped)
            continue;
        
        x_ae_context_add_level_polygon
        (
            &renderContext->renderer->activeEdgeContext,
            renderContext->level,
            level->surfaceEdgeIds + surface->firstEdgeId,
            surface->totalEdges,
            surface,
            geoFlags
        );        
    }
}

void x_bspnode_render_recursive(X_BspNode* node, X_RenderContext* renderContext, X_BspBoundBoxFrustumFlags parentNodeFlags)
{
    if(!x_bspnode_is_visible_this_frame(node, renderContext->currentFrame))
        return;
    
    X_BspBoundBoxFrustumFlags nodeFlags = x_bspboundbox_determine_frustum_clip_flags(&node->nodeBoundBox, renderContext->viewFrustum, parentNodeFlags);
    if(nodeFlags == X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
        return;
    
    if(x_bspnode_is_leaf(node))
    {
        x_bspleaf_mark_all_surfaces_in_leaf_as_visible((X_BspLeaf*)node, renderContext->currentFrame);
        return;
    }
    
    X_BspNode* frontSide;
    X_BspNode* backSide;
    x_bspnode_determine_children_sides_relative_to_camera(node, &renderContext->camPos, &frontSide, &backSide);
    
    X_BspBoundBoxFrustumFlags geoFlags = x_bspboundbox_determine_frustum_clip_flags(&node->geoBoundBox, renderContext->viewFrustum, nodeFlags);

    x_bspnode_render_recursive(frontSide, renderContext, nodeFlags);
    
    if(geoFlags != X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
        x_bspnode_render_surfaces(node, renderContext, geoFlags);
    
    x_bspnode_render_recursive(backSide, renderContext, nodeFlags);
}

void x_bsplevel_render(X_BspLevel* level, X_RenderContext* renderContext)
{
    X_BspBoundBoxFrustumFlags enableAllPlanes = (1 << renderContext->viewFrustum->totalPlanes) - 1;
    x_bspnode_render_recursive(x_bsplevel_get_level_model(level)->rootBspNode, renderContext, enableAllPlanes);
}

void x_bsplevel_get_texture(X_BspLevel* level, int textureId, int mipMapLevel, X_Texture* dest)
{
    x_assert(mipMapLevel >= 0 && mipMapLevel < 4, "Bad mip map request");
    x_assert(textureId >= 0 && textureId < level->totalTextures, "Requested invalid texture");
    
    X_BspTexture* bspTex = level->textures + textureId;
    
    dest->w = bspTex->w >> mipMapLevel;
    dest->h = bspTex->h >> mipMapLevel;
    dest->texels = bspTex->mipTexels[mipMapLevel];
}

void x_bsplevel_cleanup(X_BspLevel* level)
{
    if(!x_bsplevel_file_is_loaded(level))
        return;
    
    x_free(level->compressedPvsData);
    x_free(level->edges);
    x_free(level->faceTextures);
    x_free(level->leaves);
    x_free(level->markSurfaces);
    x_free(level->models);
    x_free(level->nodes);
    x_free(level->planes);
    x_free(level->surfaceEdgeIds);
    x_free(level->surfaces);
    x_free(level->textures);
    x_free(level->textureTexels);
    x_free(level->vertices);
}

