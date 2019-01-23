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

#include "geo/Frustum.hpp"
#include "geo/Ray3.hpp"
#include "level/BspLevel.hpp"
#include "level/BspNode.hpp"
#include "render/RenderContext.hpp"
#include "render/Renderer.hpp"

void X_BspNode::markAncestorsAsVisible(int currentFrame)
{
    auto node = this;

    do
    {
        // Don't bother walking all the way up the tree if we've already marked them as visible
        if(node->lastVisibleFrame == currentFrame)
            break;
        
        node->lastVisibleFrame = currentFrame;
        node = node->parent;
    } while(node != nullptr);
}

void X_BspLeaf::markSurfacesAsVisible(int currentFrame, int bspKey_)
{
    X_BspSurface** nextSurface = firstMarkSurface;
    
    for(int i = 0; i < totalMarkSurfaces; ++i)
    {
        X_BspSurface* surface = *nextSurface;        
        surface->lastVisibleFrame = currentFrame;
        ++nextSurface;
    }
    
    bspKey = bspKey_;
}

void X_BspNode::renderRecursive(X_RenderContext& renderContext, BoundBoxFrustumFlags parentNodeFlags)
{
    if(!isVisibleThisFrame(renderContext.currentFrame))
    {
        return;
    }
    
    BoundBoxFrustumFlags nodeFlags = nodeBoundBox.determineFrustumClipFlags(*renderContext.viewFrustum, parentNodeFlags);
    if(nodeFlags == X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM)
    {
        return;
    }
    
    if(isLeaf())
    {
        x_bsplevel_next_bspkey(renderContext.level);
        int leafBspKey = x_bsplevel_current_bspkey(renderContext.level);
        getLeaf()->markSurfacesAsVisible(renderContext.currentFrame, leafBspKey);
        return;
    }
    
    X_BspNode* frontSideRelativeToCamera;
    X_BspNode* backSideRelativeToCamera;
    
    fp distanceToPlane = plane->plane.distanceTo(renderContext.camPos);
    bool onNormalSide = distanceToPlane > 0;
    
    if(onNormalSide)
    {
        frontSideRelativeToCamera = frontChild;
        backSideRelativeToCamera = backChild;
    }
    else
    {
        frontSideRelativeToCamera = backChild;
        backSideRelativeToCamera = frontChild;
    }
    
    frontSideRelativeToCamera->renderRecursive(renderContext, nodeFlags);
    
    renderSurfaces(renderContext, nodeFlags, distanceToPlane);
    
    backSideRelativeToCamera->renderRecursive(renderContext, nodeFlags);
}

void X_BspNode::renderSurfaces(X_RenderContext& renderContext, BoundBoxFrustumFlags geoFlags, fp distanceToPlane)
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
    
    for(int i = 0; i < totalSurfaces; ++i)
    {
        X_BspSurface* surface = firstSurface + i;
        
        if(!x_bspsurface_is_visible_this_frame(surface, renderContext.currentFrame))
            continue;
        
        bool onNormalSide = distanceToPlane > 0;
        bool planeFlipped = (surface->flags & X_BSPSURFACE_FLIPPED) != 0;
        
        if((!onNormalSide) ^ planeFlipped)
            continue;
        
//        scheduleSurfaceToRender(renderContext, surface->id);
        
        renderContext.renderer->activeEdgeContext.addLevelPolygon(
            renderContext.level,
            level->surfaceEdgeIds + surface->firstEdgeId,
            surface->totalEdges,
            surface,
            geoFlags,
            x_bsplevel_current_bspkey(renderContext.level));        
    }
}

