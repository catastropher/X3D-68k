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

#include "X_BspLevel.h"
#include "X_BspLevelLoader.h"
#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"

void x_bsplevel_render_wireframe(X_BspLevel* level, X_RenderContext* rcontext, X_Color color)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        X_BspEdge* edge = level->edges + i;
        
        X_Ray3 ray = x_ray3_make
        (
            level->vertices[edge->v[0]].v,
            level->vertices[edge->v[1]].v
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
//     level->compressedPvsData = NULL;
//     
//     level->edges = NULL;
//     level->totalEdges = 0;
//     
//     level->faces = NULL;
//     level->totalFaces = 0;
//     
//     level->file.file = NULL;
//     
//     level->leaves = NULL;
//     level->totalLeaves = 0;
//     
//     level->nodes = NULL;
//     level->totalNodes = 0;
//     
//     level->planes = NULL;
//     level->totalPlanes = 0;
//     
//     level->vertices = NULL;
//     level->totalVertices = 0;
    
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
    X_BspSurface** surface = leaf->firstMarkSurface;
    
    for(int i = 0; i < leaf->totalMarkSurfaces; ++i)
    {
        (*surface)->lastVisibleFrame = currentFrame;
        ++surface;
    }
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

static void x_bspnode_render_surfaces(X_BspNode* node, X_RenderContext* renderContext)
{
    X_BspLevel* level = renderContext->level;
    
    for(int i = 0; i < node->totalSurfaces; ++i)
    {
        X_BspSurface* surface = node->firstSurface + i;
        
        if(!x_bspsurface_is_visible_this_frame(surface, renderContext->currentFrame))
            continue;
        
        for(int j = 0; j < surface->totalEdges; ++j)
        {
            int edgeId = level->surfaceEdgeIds[surface->firstEdgeId + j];
            
            X_BspEdge* edge = level->edges + abs(edgeId);
            
            X_Ray3 ray = x_ray3_make
            (
                level->vertices[edge->v[0]].v,
                level->vertices[edge->v[1]].v
            );
            
            x_ray3d_render(&ray, renderContext, 255);
        }
    }
}

void x_bspnode_render_recursive(X_BspNode* node, X_RenderContext* renderContext)
{
    if(!x_bspnode_is_visible_this_frame(node, renderContext->currentFrame))
        return;
    
    if(x_bspnode_is_leaf(node))
    {
        x_bspleaf_mark_all_surfaces_in_leaf_as_visible((X_BspLeaf*)node, renderContext->currentFrame);
        return;
    }
    
    X_BspNode* frontSide;
    X_BspNode* backSide;
    x_bspnode_determine_children_sides_relative_to_camera(node, &renderContext->camPos, &frontSide, &backSide);
    
    x_bspnode_render_recursive(frontSide, renderContext);
    x_bspnode_render_surfaces(node, renderContext);
    x_bspnode_render_recursive(backSide, renderContext);
}

void x_bsplevel_render(X_BspLevel* level, X_RenderContext* renderContext)
{
    x_bspnode_render_recursive(x_bsplevel_get_level_model(level)->rootBspNode, renderContext);
}

