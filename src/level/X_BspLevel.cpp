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

#include <new>

#include "geo/X_Ray3.h"
#include "render/X_activeedge.h"
#include "render/X_RenderContext.h"
#include "render/X_Renderer.h"
#include "X_BspLevel.h"
#include "X_BspLevelLoader.h"
#include "error/X_error.h"
#include "geo/X_BoundSphere.h"
#include "engine/X_EngineContext.h"

void X_BspLevel::renderWireframe(X_RenderContext& renderContext, X_Color color)
{
    unsigned char drawnEdges[8192];
    memset(drawnEdges, 8192, (totalEdges + 7) / 8);

    int totalPlanes = renderContext.viewFrustum->totalPlanes;

    renderContext.viewFrustum->totalPlanes = 6;

    models[0].renderWireframe(renderContext, color, drawnEdges);
    
    for(int i = 1; i < totalModels; ++i)
    {
        models[i].renderWireframe(renderContext, 15, drawnEdges);
    }

    renderContext.viewFrustum->totalPlanes = totalPlanes;
}

X_BspLeaf* X_BspLevel::findLeafPointIsIn(Vec3fp& point)
{
    X_BspNode* node = x_bsplevel_get_root_node(this);
 
    do
    {
        node = node->plane->plane.pointOnNormalFacingSide(point)
            ? node->frontChild
            : node->backChild;
    } while(!node->isLeaf());
    
    return node->getLeaf();
}

void X_BspLevel::markAllLeavesInPvsAsVisible(unsigned char* pvs, int pvsSize)
{
    memset(pvs, 0xFF, pvsSize);
}

void X_BspLevel::decompressPvs(unsigned char* compressedPvsData, int pvsSize, unsigned char* decompressedPvsDest)
{
    unsigned char* decompressedPvsEnd = decompressedPvsDest + pvsSize;
    
    while(decompressedPvsDest < decompressedPvsEnd)
    {
        if(*compressedPvsData == 0)
        {
            ++compressedPvsData;
            int count = *compressedPvsData++;
            
            for(int i = 0; i < count; ++i)
            {
                *decompressedPvsDest++ = 0;
            }
        }
        else
        {
            *decompressedPvsDest++ = *compressedPvsData++;
        }
    }
}

void X_BspLevel::decompressPvsForLeaf(X_BspLeaf* leaf, unsigned char* decompressedPvsDest)
{
    int pvsSize = x_bspfile_node_pvs_size(this);
    unsigned char* pvsData = leaf->compressedPvsData;

    bool hasVisibilityInfoForCurrentLeaf = pvsData != nullptr && !leaf->isOutsideLevel();
    
    if(!hasVisibilityInfoForCurrentLeaf)
    {
        markAllLeavesInPvsAsVisible(decompressedPvsDest, pvsSize);
        return;
    }
    
    decompressPvs(pvsData, pvsSize, decompressedPvsDest);
}

int X_BspLevel::countVisibleLeaves(unsigned char* pvs)
{
    int count = 0;
    
    for(int i = 0; i < x_bsplevel_get_level_model(this)->totalBspLeaves; ++i)
    {
        if(pvs[i / 8] & (1 << (i & 7)))
        {
            ++count;
        }
    }
    
    return count;
}

void X_BspLevel::initEmpty()
{    
    flags = (X_BspLevelFlags)0;
}

void X_BspLevel::markVisibleLeavesFromPvs(unsigned char* pvs, int currentFrame)
{
    int totalLeaves = x_bsplevel_get_level_model(this)->totalBspLeaves;
    
    for(int i = 0; i < totalLeaves; ++i)
    {
        bool leafVisible = pvs[i / 8] & (1 << (i & 7));
        X_BspNode* leafNode = (X_BspNode*)x_bsplevel_get_leaf(this, i + 1);    // PVS excludes leaf 0 so we start at leaf 1
        
        if(leafVisible)
        {
            leafNode->markAncestorsAsVisible(currentFrame);
        }
    }
}

void X_BspLevel::renderPortals(X_RenderContext& renderContext)
{
    BoundBoxFrustumFlags bbFlags = (BoundBoxFrustumFlags)((1 << 4) - 1);

    for(auto portal = portalHead; portal != nullptr; portal = portal->next)
    {
        //portal->aeSurface = renderContext.renderer->activeEdgeContext.addPortalPolygon(portal->poly, bbFlags, 0);
    }
}

// static void x_bspnode_mark_surfaces_in_node_as_close_to_light(X_BspNode* node, const X_Light* light, int currentFrame)
// {
//     for(int i = 0; i < node->totalSurfaces; ++i)
//     {
//         X_BspSurface* surface = node->firstSurface + i;
        
//         if(surface->lastLightUpdateFrame != currentFrame)
//         {
//             surface->lastLightUpdateFrame = currentFrame;
//             surface->lightsTouchingSurface = 0;
//         }
        
//         surface->lightsTouchingSurface |= (1 << light->id);
//     }
// }

// static void x_bspnode_mark_surfaces_light_is_close_to(X_BspNode* node, const X_Light* light, int currentFrame)
// {
//     if(node->isLeaf())
//         return;
    
//     Plane* plane = &node->plane->plane;

//     Vec3fp lightPosTemp = MakeVec3fp(light->position);

//     int dist = plane->distanceTo(lightPosTemp).toInt();
    
//     if(dist > light->intensity)
//     {
//         // Too far away from this node on the front side
//         x_bspnode_mark_surfaces_light_is_close_to(node->frontChild, light, currentFrame);
//         return;
//     }
    
//     if(dist < -light->intensity)
//     {
//         // Too far away from this node on the back side
//         x_bspnode_mark_surfaces_light_is_close_to(node->backChild, light, currentFrame);
//         return;
//     }
    
//     x_bspnode_mark_surfaces_in_node_as_close_to_light(node, light, currentFrame);
//     x_bspnode_mark_surfaces_light_is_close_to(node->frontChild, light, currentFrame);
//     x_bspnode_mark_surfaces_light_is_close_to(node->backChild, light, currentFrame);
// }

// void x_bsplevel_mark_surfaces_light_is_close_to(X_BspLevel* level, const X_Light* light, int currentFrame)
// {
//     x_bspnode_mark_surfaces_light_is_close_to(x_bsplevel_get_level_model(level)->rootBspNode, light, currentFrame);
// }

static void x_bsplevel_render_submodel(X_BspLevel* level, X_BspModel* submodel, X_RenderContext* renderContext, BoundBoxFrustumFlags geoFlags)
{
    x_ae_context_set_current_model(&renderContext->renderer->activeEdgeContext, submodel);
    
    for(int i = 0; i < submodel->totalFaces; ++i)
    {        
        X_BspSurface* surface = submodel->faces + i;
     
        
        // FIXME: figure out how to do backface culling for submodels, since they can move
        // This means their plane equations won't be correct
        
//         bool onNormalSide = x_plane_point_is_on_normal_facing_side(&surface->plane->plane, &renderContext->camPos);
//         bool planeFlipped = (surface->flags & X_BSPSURFACE_FLIPPED) != 0;
//         
//         if((!onNormalSide) ^ planeFlipped)
//            continue;
        
        renderContext->renderer->activeEdgeContext.addSubmodelPolygon(
            renderContext->level,
            level->surfaceEdgeIds + surface->firstEdgeId,
            surface->totalEdges,
            surface,
            geoFlags,
            x_bsplevel_current_bspkey(renderContext->level));        
    }
}

void x_bsplevel_render_submodels(X_BspLevel* level, X_RenderContext* renderContext)
{
    BoundBoxFrustumFlags enableAllPlanes = (BoundBoxFrustumFlags)((1 << renderContext->viewFrustum->totalPlanes) - 1);
    
    for(int i = 1; i < level->totalModels; ++i)
        x_bsplevel_render_submodel(level, level->models + i, renderContext, enableAllPlanes);
}

void x_bsplevel_render(X_BspLevel* level, X_RenderContext* renderContext)
{
    x_bsplevel_reset_bspkeys(level);
    x_ae_context_set_current_model(&renderContext->renderer->activeEdgeContext, x_bsplevel_get_level_model(level));
    
    BoundBoxFrustumFlags enableAllPlanes = (BoundBoxFrustumFlags)((1 << renderContext->viewFrustum->totalPlanes) - 1);
    
     if(!x_keystate_key_down(renderContext->engineContext->getKeyState(), (X_Key)'g'))
        x_bsplevel_get_level_model(level)->rootBspNode->renderRecursive(*renderContext, enableAllPlanes);
    
    x_bsplevel_render_submodels(level, renderContext);
}

void x_bsplevel_get_texture(X_BspLevel* level, int textureId, int mipMapLevel, X_Texture* dest)
{
    x_assert(mipMapLevel >= 0 && mipMapLevel < 4, "Bad mip map request");
    x_assert(textureId >= 0 && textureId < level->totalTextures, "Requested invalid texture");
    
    X_BspTexture* bspTex = level->textures + textureId;
    
    new (dest) X_Texture(bspTex->w >> mipMapLevel,  bspTex->h >> mipMapLevel, bspTex->mipTexels[mipMapLevel]);
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
    if(node->isLeaf())
        return nextNodeDest;
    
    Vec3fp sphereCenterTemp = MakeVec3fp(sphere->center);

    x_fp16x16 dist = node->plane->plane.distanceTo(sphereCenterTemp).toFp16x16();
    bool exploreFront = 1;
    bool exploreBack = 1;
    
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

Portal* X_BspLevel::addPortal()
{
    auto portal = Zone::alloc<Portal>();

    portal->next = portalHead;
    portalHead = portal;

    return portal;
}



