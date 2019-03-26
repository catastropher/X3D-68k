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

#include "geo/Ray3.hpp"
#include "render/ActiveEdge.hpp"
#include "render/RenderContext.hpp"
#include "render/OldRenderer.hpp"
#include "BspLevel.hpp"
#include "BspLevelLoader.hpp"
#include "error/Error.hpp"
#include "geo/BoundSphere.hpp"
#include "engine/EngineContext.hpp"
#include "level/Portal.hpp"
#include "entity/component/BrushModelComponent.hpp"

BspLeaf* BspLevel::findLeafPointIsIn(Vec3fp& point)
{
    BspNode* node = &getLevelRootNode();
 
    do
    {
        node = node->plane->plane.pointOnNormalFacingSide(point)
            ? node->frontChild
            : node->backChild;
    } while(!node->isLeaf());
    
    return &node->getLeaf();
}

void BspLevel::initEmpty()
{    
    flags = (X_BspLevelFlags)0;
}

void BspLevel::renderPortals(X_RenderContext& renderContext)
{
    BoundBoxFrustumFlags bbFlags = (BoundBoxFrustumFlags)((1 << 4) - 1);

    for(auto portal = portalHead; portal != nullptr; portal = portal->next)
    {
        if(!portal->plane.pointOnNormalFacingSide(renderContext.camPos))
        {
            continue;
        }

        if(portal->otherSide != nullptr)
        {
            portal->aeSurface = renderContext.renderer->activeEdgeContext.addBrushPolygon(portal->poly, portal->plane, bbFlags, 0);

            if(portal->aeSurface != nullptr)
            {
                // Disable rendering spans of the surface because we're going to render the portal in its place
                portal->aeSurface->flags.set(SURFACE_NO_DRAW_SPANS);
            }

            if(portal->flags.hasFlag(PORTAL_DRAW_OUTLINE))
            {
                Vec3fp vertices[16];
                Polygon3 outline(vertices, 16);

                portal->poly.scaleRelativeToCenter(fp::fromFloat(1.1), outline);

                auto outlineSurface = renderContext.renderer->activeEdgeContext.addBrushPolygon(outline, portal->plane, bbFlags, 0);

                if(outlineSurface != nullptr)
                {
                    outlineSurface->enableSolidFill(portal->outlineColor);
                }
            }
        }
        else
        {
            portal->aeSurface = nullptr;
        }
    }
}

// static void x_bspnode_mark_surfaces_in_node_as_close_to_light(BspNode* node, const X_Light* light, int currentFrame)
// {
//     for(int i = 0; i < node->totalSurfaces; ++i)
//     {
//         BspSurface* surface = node->firstSurface + i;
        
//         if(surface->lastLightUpdateFrame != currentFrame)
//         {
//             surface->lastLightUpdateFrame = currentFrame;
//             surface->lightsTouchingSurface = 0;
//         }
        
//         surface->lightsTouchingSurface |= (1 << light->id);
//     }
// }

// static void x_bspnode_mark_surfaces_light_is_close_to(BspNode* node, const X_Light* light, int currentFrame)
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

// void x_bsplevel_mark_surfaces_light_is_close_to(BspLevel* level, const X_Light* lights, int currentFrame)
// {
//     x_bspnode_mark_surfaces_light_is_close_to(x_bsplevel_get_level_model(level)->rootBspNode, light, currentFrame);
// }

void x_bsplevel_get_texture(BspLevel* level, int textureId, int mipMapLevel, Texture* dest)
{
    x_assert(mipMapLevel >= 0 && mipMapLevel < 4, "Bad mip map request");
    x_assert(textureId >= 0 && textureId < level->totalTextures, "Requested invalid texture");
    
    BspTexture* bspTex = level->textures + textureId;
    
    new (dest) Texture(bspTex->w >> mipMapLevel,  bspTex->h >> mipMapLevel, bspTex->mipTexels[mipMapLevel]);
}

void x_bsplevel_cleanup(BspLevel* level)
{
    if(!x_bsplevel_file_is_loaded(level))
        return;
    
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

BspNode** x_bsplevel_find_nodes_intersecting_sphere_recursive(BspNode* node, BoundSphere* sphere, BspNode** nextNodeDest)
{
    if(node->isLeaf())
    {
        return nextNodeDest;
    }

    fp dist = node->plane->plane.distanceTo(sphere->center);
    bool exploreFront = true;
    bool exploreBack = true;
    
    if(dist > sphere->radius)
    {
        exploreBack = false;
    }
    else if(dist < -sphere->radius)
    {
        exploreFront = false;
    }
    else
    {
        *nextNodeDest++ = node;
    }
    
    if(exploreFront)
    {
        nextNodeDest = x_bsplevel_find_nodes_intersecting_sphere_recursive(node->frontChild, sphere, nextNodeDest);
    }

    if(exploreBack)
    {
        nextNodeDest = x_bsplevel_find_nodes_intersecting_sphere_recursive(node->backChild, sphere, nextNodeDest);
    }

    return nextNodeDest;
}

int x_bsplevel_find_nodes_intersecting_sphere(BspLevel* level, BoundSphere* sphere, BspNode** dest)
{
    return x_bsplevel_find_nodes_intersecting_sphere_recursive(&level->getLevelModel().getRootNode(), sphere, dest) - dest;
}

Portal* BspLevel::addPortal()
{
    auto portal = Zone::alloc<Portal>();

    portal->next = portalHead;
    portal->aeSurface = nullptr;
    portal->flags.clear();

    portalHead = portal;

    return portal;
}



