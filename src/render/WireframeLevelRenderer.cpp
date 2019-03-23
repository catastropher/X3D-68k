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

#include <cstring>

#include "WireframeLevelRenderer.hpp"
#include "geo/Frustum.hpp"
#include "level/BspLevel.hpp"
#include "geo/Ray3.hpp"
#include "entity/component/BrushModelComponent.hpp"
#include "entity/system/BrushModelSystem.hpp"

#include "engine/Engine.hpp"        // TODO: remove

void WireframeLevelRenderer::render()
{
    BspLevel& level = *renderContext.level;
    int totalPlanes = renderContext.viewFrustum->totalPlanes;
    int totalEdges = level.totalEdges;
    
    memset(drawnEdges, 0, (totalEdges + 7) / 8);
    
    //renderContext.viewFrustum->totalPlanes = 6;
    
    //renderModel(level.models[0], levelColor);

    BrushModelSystem* brushModelSystem = Engine::getInstance()->brushModelSystem;

    auto& entities = brushModelSystem->getAllEntities();

    printf("Render wireframe!\n");

    for(auto& entity : entities)
    {
        BrushModelComponent* brushModelComponent = entity->getComponent<BrushModelComponent>();

        if(brushModelComponent->model != nullptr)
        {
            memset(drawnEdges, 0, (totalEdges + 7) / 8);
            renderModel(*brushModelComponent->model, modelColor);
        }
    }
    
    renderContext.viewFrustum->totalPlanes = totalPlanes;
}

void WireframeLevelRenderer::renderModel(BspModel& model, X_Color color)
{
    int flags = (1 << renderContext.viewFrustum->totalPlanes) - 1;
    
    currentModelCenter = model.center;
    currentColor = color;
    currentModel = &model;
    
    renderNode(*model.rootBspNode, flags);
}

void WireframeLevelRenderer::renderNode(BspNode& node, int parentFlags)
{    
    BoundBoxFrustumFlags nodeFlags = node.nodeBoundBox
        .determineFrustumClipFlags(*renderContext.viewFrustum, (BoundBoxFrustumFlags)parentFlags);
    
    if(nodeFlags == X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
    {
        //return;
    }
    
    nodeFlags = (BoundBoxFrustumFlags)parentFlags;
    
    if(node.isLeaf())
    {
        auto& leaf = node.getLeaf();
        
        for(int i = 0; i < leaf.totalMarkSurfaces; ++i)
        {
            BspSurface* s = leaf.firstMarkSurface[i];
            
            for(int j = 0; j < s->totalEdges; ++j)
            {
                int edgeId = abs(currentModel->surfaceEdgeIds[s->firstEdgeId + j]);
                
                if(edgeHasAlreadyBeenDrawn(edgeId))
                {
                    continue;
                }
                
                BspEdge* edge = currentModel->edges + edgeId;
                
                Ray3 ray(
                    currentModel->vertices[edge->v[0]].v,
                    currentModel->vertices[edge->v[1]].v);
                
                ray.v[0] += currentModelCenter;
                ray.v[1] += currentModelCenter;
                
                markEdgeAsAlreadyDrawn(edgeId);
                
                ray.renderShaded(renderContext, currentColor, fp::fromInt(1000));
                //ray.render(renderContext, currentColor);
            }
        }
        
        return;
    }
    
    renderNode(*node.frontChild, nodeFlags);
    renderNode(*node.backChild, nodeFlags);
}

