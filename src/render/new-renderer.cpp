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
#include <algorithm>
#include <vector>

#include "level/X_BspLevel.h"
#include "geo/X_Polygon3.h"
#include "geo/X_Polygon2.hpp"
#include "render/X_activeedge.h"

static ArenaAllocator<X_AE_Edge> edgePool(100, "NewEdgePool");

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, X_BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, x_fp16x16* closestZ);

void drawSpan(int x1, int x2, int y, X_Screen* screen, X_Color color)
{
    for(int i = x1; i < x2; ++i)
    {
        x_texture_set_texel(&screen->canvas, i, y, color);
    }
}

std::vector<X_AE_Span> spans;

void emitSpan(int x1, int x2, int y)
{
    X_AE_Span span;
    span.x1 = x1;
    span.x2 = x2;
    span.y = y;
    
    spans.push_back(span);
}

void renderLevelPolygon(LevelPolygon3* poly, X_BspSurface* surface, X_RenderContext* renderContext, X_BoundBoxFrustumFlags clipFlags)
{
    edgePool.freeAll();
    spans.clear();
    
    X_Vec2 v2d[X_POLYGON3_MAX_VERTS];
    int clippedEdgeIds[X_POLYGON3_MAX_VERTS];
    
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    x_fp16x16 closestZ;
    
    if(!projectAndClipBspPolygon(poly, renderContext, clipFlags, &poly2d, &closestZ))
        return;
    
    X_BspEdge dummyEdge;
    X_AE_Surface aeSurface;
    
    int minY = 0x7FFFFFFF;
    int maxY = -0x7FFFFFFF;
    
    // Add edges
    for(int i = 0; i < poly2d.totalVertices; ++i)
    {
        int next = (i + 1 < poly2d.totalVertices ? i + 1 : 0);
        
        X_AE_Edge* edge = edgePool.alloc();
        
        new (edge) X_AE_Edge(poly2d.vertices + i, poly2d.vertices + next, &aeSurface, &dummyEdge, renderContext->currentFrame, edgePool.begin());
        
        if(edge->isHorizontal)
        {
            edgePool.freeLast();
        }
        else
        {
            minY = std::min(minY, (int)edge->startY);
            maxY = std::max(maxY, (int)edge->endY);
        }
    }
    
    // Sort by starting y
    std::sort(edgePool.begin(), edgePool.end(), [] (const X_AE_Edge& a, const X_AE_Edge& b) -> bool
    {
        return a.startY < b.startY;
    });
    
    X_AE_Edge* nextNewEdge = edgePool.begin();
    
    std::vector<X_AE_Edge*> activeEdges;
    
    for(int y = minY; y <= maxY; ++y)
    {
        // Add new edges
        while(nextNewEdge < edgePool.end() && nextNewEdge->startY == y)
        {
            printf("New edge at %d\n", y);
            
            activeEdges.push_back(nextNewEdge);
            
            x_fp16x16 sortX = nextNewEdge->x;
            if(!nextNewEdge->isLeadingEdge)     // Make sure trailing edges sort after leading edges
                ++sortX;
            
            int pos = activeEdges.size() - 2;
            
            while(pos >= 0 && activeEdges[pos]->x > sortX)
            {
                std::swap(activeEdges[pos], activeEdges[pos + 1]);
                --pos;
            }
            
            ++nextNewEdge;
        }
        
        if(activeEdges.size() % 2 != 0)
        {
            //printf("Bad number of edges\n");
        }
        
        for(int i = 0; i < activeEdges.size() / 2; ++i)
        {
            emitSpan(activeEdges[2 * i]->x >> 16, activeEdges[2 * i + 1]->x >> 16, y);
        }
        
        // Delete finished edges
        int writePos = 0;
        for(int i = 0; i < activeEdges.size(); ++i)
        {
            if(activeEdges[i]->endY != y)
            {
                activeEdges[i]->x += activeEdges[i]->xSlope;
                activeEdges[writePos++] = activeEdges[i];
            }
        }
        
        while(activeEdges.size() > writePos)
            activeEdges.pop_back();
    }
}

void testNewRenderer(X_RenderContext* renderContext)
{
    for(auto s : spans)
    {
        drawSpan(s.x1, s.x2, s.y, renderContext->screen, renderContext->screen->palette->brightRed);
    }
}

#include "engine/X_EngineContext.h"

void cmd_draw(X_EngineContext* context, int argc, char* argv[])
{
    int surf = x_ae_context_find_surface_point_is_in(&context->renderer.activeEdgeContext, 320, 240, &context->currentLevel);
    
    if(surf == -1)
        return;
    
    x_console_printf(&context->console, "Surf: %d\n", surf);
    
    X_BspSurface* s = context->currentLevel.surfaces + surf;
    Vec3 v[X_POLYGON3_MAX_VERTS];
    int edgeIds[X_POLYGON3_MAX_VERTS];
    
    LevelPolygon3 poly(v, X_POLYGON3_MAX_VERTS, edgeIds);
    Vec3 origin(0, 0, 0);
    context->currentLevel.getLevelPolygon(s, &origin, &poly);
    
    x_console_printf(&context->console, "Verts: %d\n", poly.totalVertices);

    X_RenderContext renderContext;
    x_enginecontext_get_rendercontext_for_camera(context, context->screen.cameraListHead, &renderContext);
    
    
    renderLevelPolygon(&poly, s, &renderContext, (X_BoundBoxFrustumFlags)((1 << renderContext.viewFrustum->totalPlanes) - 1));
}



