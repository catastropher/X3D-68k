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
#include "geo/X_BoundSphere.h"

static void render_recursive(X_BspLevel* level, X_BspNode* node, X_RenderContext* renderContext, X_Color color, X_BspModel* model)
{
    if(x_bspnode_is_leaf(node))
    {
        X_BspLeaf* leaf = (X_BspLeaf*)node;
        
        for(int i = 0; i < leaf->totalMarkSurfaces; ++i)
        {
            X_BspSurface* s = leaf->firstMarkSurface[i];
            
            for(int j = 0; j < s->totalEdges; ++j)
            {
                X_BspEdge* edge = level->edges + abs(level->surfaceEdgeIds[s->firstEdgeId + j]);
                
                X_Ray3 ray = x_ray3_make
                (
                    level->vertices[edge->v[0]].v,
                    level->vertices[edge->v[1]].v
                );
                
                ray.v[0] = x_vec3_add(ray.v + 0, &model->origin);
                ray.v[1] = x_vec3_add(ray.v + 1, &model->origin);
                
                x_ray3_render(&ray, renderContext, color);
            }
        }
        
        return;
    }
    
    render_recursive(level, node->frontChild, renderContext, color, model);
    render_recursive(level, node->backChild, renderContext, color, model);
}

void x_bspmodel_render_wireframe(X_BspLevel* level, X_BspModel* model, X_RenderContext* renderContext, X_Color color)
{
    render_recursive(level, model->rootBspNode, renderContext, color, model);
}

void x_bsplevel_render_wireframe(X_BspLevel* level, X_RenderContext* rcontext, X_Color color)
{
    x_bspmodel_render_wireframe(level, level->models + 0, rcontext, color);
    
    for(int i = 1; i < level->totalModels; ++i)
        x_bspmodel_render_wireframe(level, level->models + i, rcontext, 15);
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

static void mark_all_leaves_in_pvs_as_visible(unsigned char* pvs, int pvsSize)
{
    for(int i = 0; i < pvsSize; ++i)
        pvs[i] = 0xFF;
}

static void decompress_pvs_using_run_length_encoding(unsigned char* compressedPvsData, int pvsSize, unsigned char* decompressedPvsDest)
{
    unsigned char* decompressedPvsEnd = decompressedPvsDest + pvsSize;
    
    while(decompressedPvsDest < decompressedPvsEnd)
    {
        if(*compressedPvsData == 0)
        {
            ++compressedPvsData;
            int count = *compressedPvsData++;
            
            for(int i = 0; i < count; ++i)
                *decompressedPvsDest++ = 0;
        }
        else
        {
            *decompressedPvsDest++ = *compressedPvsData++;
        }
    }
}

void x_bsplevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLeaf* leaf, unsigned char* decompressedPvsDest)
{
    int pvsSize = x_bspfile_node_pvs_size(level);
    unsigned char* pvsData = leaf->compressedPvsData;

    _Bool outsideLevel = leaf == level->leaves + 0;
    _Bool hasVisibilityInfoForCurrentLeaf = pvsData != NULL && !outsideLevel;
    
    if(!hasVisibilityInfoForCurrentLeaf)
    {
        mark_all_leaves_in_pvs_as_visible(decompressedPvsDest, pvsSize);
        return;
    }
    
    decompress_pvs_using_run_length_encoding(pvsData, pvsSize, decompressedPvsDest);
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

static void x_bspleaf_mark_all_surfaces_in_leaf_as_visible(X_BspLeaf* leaf, int currentFrame, int bspKey)
{
    X_BspSurface** nextSurface = leaf->firstMarkSurface;
    
    for(int i = 0; i < leaf->totalMarkSurfaces; ++i)
    {
        X_BspSurface* surface = *nextSurface;        
        surface->lastVisibleFrame = currentFrame;
        ++nextSurface;
    }
    
    leaf->bspKey = bspKey;
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

static void x_bspnode_render_surfaces(X_BspNode* node, X_RenderContext* renderContext, X_BoundBoxFrustumFlags geoFlags)
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
            geoFlags,
            x_bsplevel_current_bspkey(renderContext->level)
        );        
    }
}

void x_bspnode_render_recursive(X_BspNode* node, X_RenderContext* renderContext, X_BoundBoxFrustumFlags parentNodeFlags)
{
    if(!x_bspnode_is_visible_this_frame(node, renderContext->currentFrame))
        return;
    
    X_BoundBoxFrustumFlags nodeFlags = x_boundbox_determine_frustum_clip_flags(&node->nodeBoundBox, renderContext->viewFrustum, parentNodeFlags);
    if(nodeFlags == X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
        return;
    
    if(x_bspnode_is_leaf(node))
    {
        x_bsplevel_next_bspkey(renderContext->level);
        int leafBspKey = x_bsplevel_current_bspkey(renderContext->level);
        x_bspleaf_mark_all_surfaces_in_leaf_as_visible((X_BspLeaf*)node, renderContext->currentFrame, leafBspKey);
        return;
    }
    
    X_BspNode* frontSide;
    X_BspNode* backSide;
    x_bspnode_determine_children_sides_relative_to_camera(node, &renderContext->camPos, &frontSide, &backSide);
    
    X_BoundBoxFrustumFlags geoFlags = x_boundbox_determine_frustum_clip_flags(&node->geoBoundBox, renderContext->viewFrustum, nodeFlags);

    x_bspnode_render_recursive(frontSide, renderContext, nodeFlags);
    
    if(geoFlags != X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
        x_bspnode_render_surfaces(node, renderContext, geoFlags);
    
    x_bspnode_render_recursive(backSide, renderContext, nodeFlags);
}

void x_bsplevel_render(X_BspLevel* level, X_RenderContext* renderContext)
{
    x_bsplevel_reset_bspkeys(level);
    
    X_BoundBoxFrustumFlags enableAllPlanes = (1 << renderContext->viewFrustum->totalPlanes) - 1;
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
    x_free(level->lightmapData);
    x_free(level->markSurfaces);
    x_free(level->models);
    x_free(level->nodes);
    x_free(level->planes);
    x_free(level->surfaceEdgeIds);
    x_free(level->surfaces);
    x_free(level->textures);
    x_free(level->textureTexels);
    x_free(level->vertices);
    x_free(level->clipNodes);
}

X_BspNode** x_bsplevel_find_nodes_intersecting_sphere_recursive(X_BspNode* node, X_BoundSphere* sphere, X_BspNode** nextNodeDest)
{
    if(x_bspnode_is_leaf(node))
        return nextNodeDest;
    
    x_fp16x16 dist = x_plane_point_distance_fp16x16(&node->plane->plane, &sphere->center);
    _Bool exploreFront = 1;
    _Bool exploreBack = 1;
    
    if(dist > sphere->radius)
        exploreBack = 0;
    else if(dist < -sphere->radius)
        exploreFront = 0;
    else
        *nextNodeDest++ = node;
    
    if(exploreFront)
        nextNodeDest = x_bsplevel_find_nodes_intersecting_sphere_recursive(node->frontChild, sphere, nextNodeDest);
    
    if(exploreBack)
        nextNodeDest = x_bsplevel_find_nodes_intersecting_sphere_recursive(node->backChild, sphere, nextNodeDest);
    
    return nextNodeDest;
}

int x_bsplevel_find_nodes_intersecting_sphere(X_BspLevel* level, X_BoundSphere* sphere, X_BspNode** dest)
{
    return x_bsplevel_find_nodes_intersecting_sphere_recursive(x_bsplevel_get_root_node(level), sphere, dest) - dest;
}



