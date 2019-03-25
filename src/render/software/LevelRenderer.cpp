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

#include <entity/system/BrushModelSystem.hpp>
#include "LevelRenderer.hpp"
#include "geo/Frustum.hpp"
#include "entity/component/BrushModelComponent.hpp"
#include "render/OldRenderer.hpp"
#include "engine/Engine.hpp"

void LevelRenderer::render(const X_RenderContext& renderContext)
{
    BspModel& levelModel = renderContext.level->getLevelModel();
    BspNode& rootLevelNode = levelModel.getRootNode();

    nextBspKey = 0;

    x_ae_context_set_current_model(&renderContext.renderer->activeEdgeContext, &levelModel);

    BoundBoxFrustumFlags enableAllPlanes = (BoundBoxFrustumFlags)((1 << renderContext.viewFrustum->totalPlanes) - 1);

    renderRecursive(rootLevelNode, renderContext, enableAllPlanes);
    renderBrushModels(renderContext);
}

void LevelRenderer::renderRecursive(BspNode& node, const X_RenderContext& renderContext, BoundBoxFrustumFlags parentNodeFlags)
{
    if(!node.isVisibleThisFrame(renderContext.currentFrame))
    {
        return;
    }

    BoundBoxFrustumFlags nodeFlags = node.nodeBoundBox.determineFrustumClipFlags(*renderContext.viewFrustum, parentNodeFlags);
    if(nodeFlags == X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
    {
        return;
    }

    if(node.isLeaf())
    {
        int leafBspKey = ++nextBspKey;

        BspLeaf& leaf = node.getLeaf();
        markSurfacesAsVisible(leaf, renderContext.currentFrame, leafBspKey);
    }
    else
    {
        BspNode* frontSideRelativeToCamera;
        BspNode* backSideRelativeToCamera;

        fp distanceToPlane = node.plane->plane.distanceTo(renderContext.camPos);
        bool onNormalSide = distanceToPlane > 0;

        if(onNormalSide)
        {
            frontSideRelativeToCamera = node.frontChild;
            backSideRelativeToCamera = node.backChild;
        }
        else
        {
            frontSideRelativeToCamera = node.backChild;
            backSideRelativeToCamera = node.frontChild;
        }

        renderRecursive(*frontSideRelativeToCamera, renderContext, nodeFlags);
        renderSurfaces(node, renderContext, nodeFlags, distanceToPlane);
        renderRecursive(*backSideRelativeToCamera, renderContext, nodeFlags);
    }
}

void LevelRenderer::renderSurfaces(const BspNode& node, const X_RenderContext& renderContext, BoundBoxFrustumFlags geoFlags, fp distanceToPlane)
{
    // Stitch the frustum planes together
    X_Frustum* frustum = renderContext.viewFrustum;
    int flags = (int)geoFlags;
    FrustumPlane* planePtr = nullptr;

    for(int i = frustum->totalPlanes - 1; i >= 0; --i)
    {
        if(flags & (1 << i))
        {
            frustum->planes[i].next = planePtr;
            planePtr = &frustum->planes[i];
        }
    }

    frustum->head = planePtr;

    BspLevel* level = renderContext.level;

    for(int i = 0; i < node.totalSurfaces; ++i)
    {
        BspSurface* surface = node.firstSurface + i;

        if(!x_bspsurface_is_visible_this_frame(surface, renderContext.currentFrame))
            continue;

        bool onNormalSide = distanceToPlane > 0;
        bool planeFlipped = (surface->flags & X_BSPSURFACE_FLIPPED) != 0;

        if((!onNormalSide) ^ planeFlipped)
            continue;

        renderContext.renderer->activeEdgeContext.addLevelPolygon(
            renderContext.level,
            level->surfaceEdgeIds + surface->firstEdgeId,
            surface->totalEdges,
            surface,
            geoFlags,
            nextBspKey);
    }
}

void LevelRenderer::markSurfacesAsVisible(BspLeaf& leaf, int currentFrame, int leafBspKey)
{
    BspSurface** nextSurface = leaf.firstMarkSurface;

    for(int i = 0; i < leaf.totalMarkSurfaces; ++i)
    {
        BspSurface* surface = *nextSurface;
        surface->lastVisibleFrame = currentFrame;
        ++nextSurface;
    }

    leaf.bspKey = leafBspKey;
}

void LevelRenderer::renderBrushModels(const X_RenderContext& renderContext)
{
    BoundBoxFrustumFlags enableAllPlanes = (BoundBoxFrustumFlags)((1 << renderContext.viewFrustum->totalPlanes) - 1);

    BrushModelSystem* brushModelSystem = Engine::getInstance()->brushModelSystem;

    auto& allBrushModels = brushModelSystem->getAllEntities();

    for(Entity* entity : allBrushModels)
    {
        BrushModelComponent* brushModelComponent = entity->getComponent<BrushModelComponent>();

        if(brushModelComponent->model != nullptr)
        {
            // FIXME: need a way to exclude the level model
            if(brushModelComponent->model == &renderContext.level->getLevelModel())
            {
                continue;
            }

            renderBrushModel(*brushModelComponent->model, renderContext, enableAllPlanes);
        }
    }
}

void LevelRenderer::renderBrushModel(BspModel& brushModel, const X_RenderContext& renderContext, BoundBoxFrustumFlags geoFlags)
{
    printf("Render brush model\n");
    x_ae_context_set_current_model(&renderContext.renderer->activeEdgeContext, &brushModel);

    for(int i = 0; i < brushModel.totalFaces; ++i)
    {
        BspSurface* surface = brushModel.faces + i;


        // FIXME: figure out how to do backface culling for submodels, since they can move
        // This means their plane equations won't be correct

//         bool onNormalSide = x_plane_point_is_on_normal_facing_side(&surface->plane->plane, &renderContext->camPos);
//         bool planeFlipped = (surface->flags & X_BSPSURFACE_FLIPPED) != 0;
//
//         if((!onNormalSide) ^ planeFlipped)
//            continue;

        renderContext.renderer->activeEdgeContext.addSubmodelPolygon(
            renderContext.level,
            brushModel.surfaceEdgeIds + surface->firstEdgeId,
            surface->totalEdges,
            surface,
            geoFlags,
            nextBspKey);
    }
}

