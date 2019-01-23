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

#include "level/BspLevel.hpp"
#include "geo/Polygon3.hpp"
#include "geo/Polygon2.hpp"
#include "render/ActiveEdge.hpp"

static ArenaAllocator<X_AE_Edge> edgePool(1000, "NewEdgePool");

#define SCREEN_W 640
#define SCREEN_H 480

#define MAX_SPANS 2000

X_AE_Span clippedSpans[MAX_SPANS];
X_AE_Span* currentClippedSpan;

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, x_fp16x16* closestZ);

void drawSpan(int x1, int x2, int y, X_Screen* screen, X_Color color)
{
    for(int i = x1; i <= x2; ++i)
    {
        screen->canvas.setTexel({ i, y }, color);
    }
}

void clipSpan(int left, int right, int y);

struct Span
{
    Span(short left_, short right_) : left(left_), right(right_) { }
    Span() { }
    
    short left;
    short right;
};

const int STRIDE = 32;

int totalCSpans[480];
Span cspans[480 * STRIDE];

Span* findFirstCSpanAfter(int left, int y, Span* cspans, int* totalCSpans)
{
    Span* span = &cspans[y * STRIDE];
    
    if(span[totalCSpans[y] / 2].left <= left)
        span = &span[totalCSpans[y] / 2];
    
    do
    {
        ++span;
    } while(span->left <= left);
    
    return span - 1;
}

#include <iostream>

inline X_AE_Span* __attribute__((always_inline))  clipSpan2(short left, short right, int y, X_AE_Span* drawSpans, Span* cspans, int* totalCSpans)
{
    if(left > right)
        return drawSpans;
    
    Span* span = findFirstCSpanAfter(left, y, cspans, totalCSpans);
    Span* spanEnd = &cspans[y * STRIDE + totalCSpans[y]];
    
    Span* spanptr;
    
    if(span->right >= left - 1)
    {
        // The new span is fully clipped by this span
        if(right <= span->right)
            return drawSpans;
        
        // We can merge with the previous span
        spanptr = span;
    }
    else if(right >= span[1].left - 1)
    {
        // We can merge with the next span
        drawSpans->x1 = left;
        drawSpans->x2 = span[1].left - 1;
        drawSpans->y = y;
        ++drawSpans;
        
        spanptr = span + 1;
    }
    else
    {
        drawSpans->x1 = left;
        drawSpans->x2 = right;
        drawSpans->y = y;
        ++drawSpans;
        
        // Have no choice but to insert :(
        for(Span* s = &cspans[y * STRIDE + totalCSpans[y]]; s > span; --s)
            *s = *(s - 1);
        
        *(span + 1) = Span(left, right);
        
        ++totalCSpans[y];
        
        return drawSpans;
    }
    
    // Merge with adjacent spans that we overlap with
    Span* mergeSpan = spanptr + 1;
    
    
    while(right >= mergeSpan->left - 1)
    {
        right = std::max(right, mergeSpan->right);
        --totalCSpans[y];
        
        drawSpans->x1 = mergeSpan[-1].right + 1;
        drawSpans->x2 = mergeSpan->left - 1;
        drawSpans->y = y;
        ++drawSpans;
        
        ++mergeSpan;
    }
    
    if(mergeSpan[-1].right + 1 <= right)
    {
        drawSpans->x1 = mergeSpan[-1].right + 1;
        drawSpans->x2 = right;
        drawSpans->y = y;
        ++drawSpans;
    }
    
    spanptr->left = std::min(spanptr->left, left);
    spanptr->right = std::max(spanptr->right, right);
    
    if(mergeSpan == spanptr + 1)
        return drawSpans;
    
    while(mergeSpan < spanEnd)
    {
        ++spanptr;
        *spanptr = *mergeSpan++;
    }
    
    return drawSpans;
}

X_AE_Edge* projectPoly(LevelPolygon3* poly, LevelPolygon2* dest, X_RenderContext* renderContext, BoundBoxFrustumFlags clipFlags, x_fp16x16* minZ, int* minY, int* maxY)
{
    edgePool.freeAll();
    
    x_fp16x16 closestZ;
    
    if(!projectAndClipBspPolygon(poly, renderContext, clipFlags, dest, &closestZ))
        return NULL;
    
    *minZ = closestZ;
    
    X_BspEdge dummyEdge;
    X_AE_Surface aeSurface;
    
    *minY = 0x7FFFFFFF;
    *maxY = -0x7FFFFFFF;
    
    // Add edges
    for(int i = 0; i < dest->totalVertices; ++i)
    {
        int next = (i + 1 < dest->totalVertices ? i + 1 : 0);
        
        X_AE_Edge* edge = edgePool.alloc();
        
        new (edge) X_AE_Edge(dest->vertices + i, dest->vertices + next, &aeSurface, &dummyEdge, renderContext->currentFrame, edgePool.begin());
        
        if(edge->isHorizontal)
        {
            edgePool.freeLast();
        }
        else
        {
            *minY = std::min(*minY, (int)edge->startY);
            *maxY = std::max(*maxY, (int)edge->endY);
        }
    }
    
    // Sort by starting y
    std::sort(edgePool.begin(), edgePool.end(), [] (const X_AE_Edge& a, const X_AE_Edge& b) -> bool
    {
        return a.startY < b.startY;
    });
    
    edgePool.alloc()->startY = *maxY + 100;
    
    return edgePool.begin();
}

X_AE_Span* __attribute__((hot)) renderConvexPolygon(LevelPolygon2* poly, X_AE_Edge* sortedEdges, int minY, int maxY, X_AE_Span* spans)
{
    x_fp16x16 leftX = 0;
    x_fp16x16 leftSlope = 0;
    
    x_fp16x16 rightX = 0;
    x_fp16x16 rightSlope = 0;
    
    int nextEdgeY;
    
    int y = minY;
    
    Span* clipSpans = cspans;
    int* totalClipSpans = totalCSpans;
    
    do
    {
        while(sortedEdges->startY == y)
        {
            if(sortedEdges->isLeadingEdge)
            {
                leftX = sortedEdges->x;
                leftSlope = sortedEdges->xSlope;
            }
            else
            {
                rightX = sortedEdges->x;
                rightSlope = sortedEdges->xSlope;
            }
            
            ++sortedEdges;
        }
        
        nextEdgeY = sortedEdges->startY;
        
        while(y < std::min(nextEdgeY, maxY))
        {
            spans = clipSpan2(leftX >> 16, rightX >> 16, y, spans, clipSpans, totalClipSpans);
            
            leftX += leftSlope;
            rightX += rightSlope;
            
            ++y;
        }
    } while(y < maxY);
    
    return spans;
}

int wrap(int num, int max)
{
    return num >= max ? num - max : num;
}

bool polygon2IsConvex(Polygon2* poly)
{
    int sign;
    
    for(int i = 0; i < poly->totalVertices; ++i)
    {
        int next = wrap(i + 1, poly->totalVertices);
        int nextNext = wrap(i + 2, poly->totalVertices);
        
        X_Vec2 b = poly->vertices[next] - poly->vertices[i];
        X_Vec2 a = poly->vertices[nextNext] - poly->vertices[next];
        
        x_fp16x16 zCross = (a.x >> 16) * (b.y >> 16) - (a.y >> 16) * (b.x >> 16);
        
        if(i == 0)
            sign = zCross > 0;
        else if(sign != (zCross > 0))
            return false;
    }
    
    return true;
}

X_AE_Span* __attribute__((hot)) renderLevelPolygon(LevelPolygon3* poly, X_BspSurface* surface, X_RenderContext* renderContext, BoundBoxFrustumFlags clipFlags, x_fp16x16* minZ, X_AE_Span* spans) 
{
    X_Vec2 v2d[X_POLYGON3_MAX_VERTS];
    int clippedEdgeIds[X_POLYGON3_MAX_VERTS];
    
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    
    int minY;
    int maxY;
    
    X_AE_Edge* nextNewEdge = projectPoly(poly, &poly2d, renderContext, clipFlags, minZ, &minY, &maxY);
    if(!nextNewEdge)
        return spans;
    
    if(polygon2IsConvex(&poly2d))
    {
        return renderConvexPolygon(&poly2d, nextNewEdge, minY,  maxY, spans);
    }
    
    Span* clipSpans = cspans;
    int* totalClipSpans = totalCSpans;
    
    X_AE_Edge* activeEdges[10];
    int totalActiveEdges = 0;
    
    for(int y = minY; y <= maxY; ++y)
    {
        // Add new edges
        while(nextNewEdge < edgePool.end() - 1 && nextNewEdge->startY == y)
        {
            activeEdges[totalActiveEdges++] = nextNewEdge;
            
            x_fp16x16 sortX = nextNewEdge->x;
            if(!nextNewEdge->isLeadingEdge)     // Make sure trailing edges sort after leading edges
                ++sortX;
            
            int pos = totalActiveEdges - 2;
            
            while(pos >= 0 && activeEdges[pos]->x > sortX)
            {
                std::swap(activeEdges[pos], activeEdges[pos + 1]);
                --pos;
            }
            
            ++nextNewEdge;
        }
        
        for(int i = 0; i < totalActiveEdges / 2; ++i)
        {
            spans = clipSpan2(activeEdges[2 * i]->x >> 16, activeEdges[2 * i + 1]->x >> 16, y, spans, clipSpans, totalClipSpans);
        }
        
        // Delete finished edges
        int writePos = 0;
        for(int i = 0; i < totalActiveEdges; ++i)
        {
            if(activeEdges[i]->endY != y)
            {
                activeEdges[i]->x += activeEdges[i]->xSlope;
                activeEdges[writePos++] = activeEdges[i];
            }
        }
        
        totalActiveEdges = writePos;
    }
    
    return spans;
}

void testNewRenderer(X_RenderContext* renderContext)
{
    return;
}

#include "engine/EngineContext.hpp"
#include <object/CameraObject.hpp>

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
    
    
    //renderLevelPolygon(&poly, s, &renderContext, (BoundBoxFrustumFlags)((1 << renderContext.viewFrustum->totalPlanes) - 1));
}

int surfacesToRender[2000];
int totalSurfaceToRender;
int currentFrame = -1;

void scheduleSurfaceToRender(X_RenderContext* renderContext, int surface)
{
    if(renderContext->currentFrame != currentFrame)
    {
        totalSurfaceToRender = 0;
        currentFrame = renderContext->currentFrame;
        
        for(int i = 0; i < 480; ++i)
        {
            cspans[i * STRIDE + 0] = Span(-5, -5);
            cspans[i * STRIDE + 1] = Span(1000, 1000);
            totalCSpans[i] = 2;
        }
    }
    
    surfacesToRender[totalSurfaceToRender++] = surface;
}

unsigned int cbuffer[SCREEN_H * SCREEN_W / 32];

typedef unsigned int uint;

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

void stitchSpans(X_AE_Span* end)
{
    for(X_AE_Span* span = clippedSpans; span < end; ++span)
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
    
    //currentClippedSpan = clippedSpans;
    
    X_AE_Span* spanEnd = renderLevelPolygon(&poly, s, renderContext, (BoundBoxFrustumFlags)((1 << renderContext->viewFrustum->totalPlanes) - 1), &aeSurface.closestZ, clippedSpans);

    if(spanEnd == clippedSpans)
        return;
    
    //if(currentClippedSpan == clippedSpans)
    //    return;
    
    stitchSpans(spanEnd);
    
    aeSurface.spanHead.next = &clippedSpans[0];
    aeSurface.last = spanEnd - 1;
    
    aeSurface.calculateInverseZGradient(&camPos, &renderContext->cam->viewport, renderContext->viewMatrix, &firstVertex);
    
    X_AE_SurfaceRenderContext surfaceRenderContext;
    x_ae_surfacerendercontext_init(&surfaceRenderContext, &aeSurface, renderContext, 0);
    x_ae_surfacerendercontext_render_spans(&surfaceRenderContext);
}

void renderSurfaces(X_RenderContext* renderContext)
{
    //std::reverse(surfacesToRender, surfacesToRender + totalSurfaceToRender);
    //memset(cbuffer, 0, sizeof(cbuffer));
    
    for(int i = 0; i < totalSurfaceToRender; ++i)
    {
        renderSurface(renderContext, surfacesToRender[i]);
    }
}



