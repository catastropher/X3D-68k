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

#define SCREEN_W 320
#define SCREEN_H 240

#define MAX_SPANS 1000

X_AE_Span clippedSpans[MAX_SPANS];
X_AE_Span* currentClippedSpan;

X_AE_Span spans[MAX_SPANS];
X_AE_Span* currentSpan;

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, X_BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, x_fp16x16* closestZ);

void drawSpan(int x1, int x2, int y, X_Screen* screen, X_Color color)
{
    for(int i = x1; i <= x2; ++i)
    {
        x_texture_set_texel(&screen->canvas, i, y, color);
    }
}



void emitSpan(int x1, int x2, int y)
{
    currentSpan->x1 = x1;
    currentSpan->x2 = x2;
    currentSpan->y = y;
    
    ++currentSpan;
}

void renderLevelPolygon(LevelPolygon3* poly, X_BspSurface* surface, X_RenderContext* renderContext, X_BoundBoxFrustumFlags clipFlags, x_fp16x16* minZ)
{
    edgePool.freeAll();
    currentSpan = spans;
    
    X_Vec2 v2d[X_POLYGON3_MAX_VERTS];
    int clippedEdgeIds[X_POLYGON3_MAX_VERTS];
    
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    x_fp16x16 closestZ;
    
    if(!projectAndClipBspPolygon(poly, renderContext, clipFlags, &poly2d, &closestZ))
        return;
    
    *minZ = closestZ;
    
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
    return;
}

#include "engine/X_EngineContext.h"
#include <object/X_CameraObject.h>

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
    
    
    //renderLevelPolygon(&poly, s, &renderContext, (X_BoundBoxFrustumFlags)((1 << renderContext.viewFrustum->totalPlanes) - 1));
}

std::vector<int> surfacesToRender;
int currentFrame;

void scheduleSurfaceToRender(X_RenderContext* renderContext, int surface)
{
    if(renderContext->currentFrame != currentFrame)
    {
        surfacesToRender.clear();
        currentFrame = renderContext->currentFrame;
    }
    
    surfacesToRender.push_back(surface);
}

unsigned int cbuffer[SCREEN_H * SCREEN_W / 32];



void clipSpan(int left, int right, int y)
{    
    int leftBit = left & 31;
    int rightBit = right & 31;

    int leftWord = left / 32;
    int rightWord = right / 32;

    uint leftMask = 0xFFFFFFFF >> leftBit;
    uint rightMask = 0xFFFFFFFF << (31 - rightBit);

    uint spanMask = (leftWord == rightWord ? leftMask & rightMask : leftMask);

    int word = leftWord;
    int offset = leftWord * 32;

    bool hasSpan = false;
    
    unsigned int* cbuf = cbuffer + y * SCREEN_W / 32;

    while(word <= rightWord)
    {
        uint mask = spanMask & (~cbuf[word]);
        bool extendLastSpan = (mask & 1) != 0;

        cbuf[word] |= spanMask;     // This could also be mask

        while(mask != 0)
        {
            int start = __builtin_clz(mask);
            int end = __builtin_clz((~mask) & (0xFFFFFFFF >> start)) - 1;

            if(end == 30 && extendLastSpan)
                ++end;

            if(hasSpan && currentClippedSpan->x2 == offset + start - 1)
            {
                currentClippedSpan->x2 = end + offset;
            }
            else
            {
                ++currentClippedSpan;
                
                currentClippedSpan->x1 = start + offset;
                currentClippedSpan->x2 = end + offset;
                currentClippedSpan->y = y;
                
                //emitSpan(start + offset, end + offset);
                hasSpan = true;
            }

            mask &= 0b01111111111111111111111111111111 >> end;
        }

        ++word;

        if(word == rightWord)
            spanMask = rightMask;
        else
            spanMask = 0xFFFFFFFF;

        offset += 32;
    }
}

void stitchSpans()
{
    for(X_AE_Span* span = clippedSpans + 1; span < currentClippedSpan; ++span)
    {
        span->next = span + 1;
        ++span->x2;
    }
}

void renderSurface(X_RenderContext* renderContext, int surfaceId)
{
    X_AE_Surface aeSurface;
    
    
    X_BspSurface* s = renderContext->level->surfaces + surfaceId;
    Vec3 v[X_POLYGON3_MAX_VERTS];
    int edgeIds[X_POLYGON3_MAX_VERTS];
    
    LevelPolygon3 poly(v, X_POLYGON3_MAX_VERTS, edgeIds);
    Vec3 origin(0, 0, 0);
    renderContext->level->getLevelPolygon(s, &origin, &poly);
    
    
    Vec3 firstVertex = poly.vertices[0];
    Vec3 camPos = x_cameraobject_get_position(renderContext->cam);
    
    aeSurface.bspKey = -1;
    aeSurface.bspSurface = s;
    aeSurface.crossCount = 0;
    aeSurface.closestZ = 0x7FFFFFFF;
    
    
    aeSurface.modelOrigin = &origin;//&context->currentModel->origin;
    
    aeSurface.parent = &aeSurface;
    aeSurface.inSubmodel = false;
    
    renderLevelPolygon(&poly, s, renderContext, (X_BoundBoxFrustumFlags)((1 << renderContext->viewFrustum->totalPlanes) - 1), &aeSurface.closestZ);
    
    currentClippedSpan = clippedSpans;
    
    for(X_AE_Span* span = spans; span < currentSpan; ++span)
    {
        span->x1 = std::max(0, span->x1);
        span->x2 = std::min(SCREEN_W - 1, span->x2);
        
        clipSpan(span->x1, span->x2, span->y);
    }

    if(currentClippedSpan == clippedSpans)
        return;
    
    stitchSpans();
    aeSurface.spanHead.next = &clippedSpans[1];
    aeSurface.last = currentClippedSpan;
    
    aeSurface.calculateInverseZGradient(&camPos, &renderContext->cam->viewport, renderContext->viewMatrix, &firstVertex);
    
    X_AE_SurfaceRenderContext surfaceRenderContext;
    x_ae_surfacerendercontext_init(&surfaceRenderContext, &aeSurface, renderContext, 0);
    x_ae_surfacerendercontext_render_spans(&surfaceRenderContext);
    
    
    for(auto span : clippedSpans)
    {
        //drawSpan(span.x1, span.x2, span.y, renderContext->screen, surfaceId % 256);
    }
}

void renderSurfaces(X_RenderContext* renderContext)
{
    //std::reverse(surfacesToRender.begin(), surfacesToRender.end());
    memset(cbuffer, 0, sizeof(cbuffer));
    
    for(int s : surfacesToRender)
    {
        renderSurface(renderContext, s);
    }
}



