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

#pragma once

#include <new>

#include "math/X_fix.h"
#include "render/X_Texture.h"
#include "render/X_RenderContext.h"
#include "X_Screen.h"
#include "level/X_BspLevel.h"
#include "X_span.h"
#include "X_Viewport.h"

#include "memory/X_ArenaAllocator.hpp"

#define X_AE_SURFACE_MAX_SPANS 332

struct X_AE_Surface
{
    x_fp16x16 inverseZAtScreenPoint(x_fp16x16 x, int y) const
    {
        return x_fp16x16_mul(x, zInverseXStep) + y * zInverseYStep + zInverseOrigin;
    }
    
    bool isCloserThan(const X_AE_Surface* surface, x_fp16x16 x, int y) const
    {
        if(bspKey != surface->bspKey)
            return bspKey < surface->bspKey;
        
        if(inSubmodel ^ surface->inSubmodel)
            return inSubmodel;
        
        return inverseZAtScreenPoint(x + X_FP16x16_ONE * 7, y) >= surface->inverseZAtScreenPoint(x + X_FP16x16_ONE * 7 , y);
    }
    
    void calculateInverseZGradient(Vec3* camPos, Viewport* viewport, Mat4x4* viewMatrix, Vec3* pointOnSurface)
    {
        Plane planeInViewSpace;
        bspSurface->calculatePlaneInViewSpace(camPos, viewMatrix, pointOnSurface, &planeInViewSpace);
        
        int dist = -planeInViewSpace.d.internalValue();
        int scale = viewport->distToNearPlane;
        
        if(dist == 0 || scale == 0) return;
        
        //x_fp16x16 invDistTimesScale = //x_fp16x16_div(X_FP16x16_ONE << 10, distTimesScale) >> 6;
        
        x_fp16x16 invDist = x_fp16x16_div(X_FP16x16_ONE << 10, dist);
        x_fp16x16 invScale = (1 << 26) / scale;
        
        x_fp16x16 invDistTimesScale = x_fp16x16_mul(invDist, invScale) >> 10;
        
        zInverseXStep = x_fp16x16_mul(invDistTimesScale, planeInViewSpace.normal.x.toFp16x16());
        zInverseYStep = x_fp16x16_mul(invDistTimesScale, planeInViewSpace.normal.y.toFp16x16());
        
        int centerX = viewport->screenPos.x + viewport->w / 2;
        int centerY = viewport->screenPos.y + viewport->h / 2;
        
        zInverseOrigin = x_fp16x16_mul(planeInViewSpace.normal.z.toFp16x16(), invDist) -
            centerX * zInverseXStep -
            centerY * zInverseYStep;
    }
    
    int bspKey;
    int id;
    int crossCount;
    int xStart;
    
    X_BspSurface* bspSurface;
    X_AE_Span spanHead;
    X_AE_Span* last;
    
    x_fp16x16 zInverseXStep;
    x_fp16x16 zInverseYStep;
    x_fp16x16 zInverseOrigin;
    
    x_fp16x16 closestZ;
    
    bool inSubmodel;
    
    Vec3* modelOrigin;
    
    struct X_AE_Surface* next;
    struct X_AE_Surface* prev;
    
    struct X_AE_Surface* parent;    
};

#define X_AE_EDGE_LEFT_SURFACE 0
#define X_AE_EDGE_RIGHT_SURFACE 1

typedef struct X_AE_Edge
{
    X_AE_Edge() { }
    
    X_AE_Edge(X_Vec2_fp16x16* a, X_Vec2_fp16x16* b, X_AE_Surface* surface, X_BspEdge* bspEdge_, int currentFrame, X_AE_Edge* edgeStart)
    {
        int aY = x_fp16x16_ceil(a->y) >> 16;
        int bY = x_fp16x16_ceil(b->y) >> 16;
        
        int height = bY - aY;
        
        isHorizontal = (height == 0);
        if(isHorizontal)
            return;
        
        isLeadingEdge = (height < 0);
        
        if(isLeadingEdge)
            X_SWAP(a, b);
        
        surfaces[X_AE_EDGE_RIGHT_SURFACE] = NULL;
        surfaces[X_AE_EDGE_LEFT_SURFACE] = NULL;
        
        if(isLeadingEdge)
            surfaces[X_AE_EDGE_RIGHT_SURFACE] = surface;
        else
            surfaces[X_AE_EDGE_LEFT_SURFACE] = surface;
        
        frameCreated = currentFrame;
        bspEdge = bspEdge_;
        
        bspEdge->cachedEdgeOffset = (unsigned char*)this - (unsigned char*)edgeStart;
        
        initDeltas(a, b);
        
        startY = x_fp16x16_to_int(x_fp16x16_ceil(a->y));
        endY = x_fp16x16_to_int(x_fp16x16_ceil(b->y)) - 1;
    }
    
    void initDeltas(X_Vec2_fp16x16* top, X_Vec2_fp16x16* bottom)
    {
        xSlope = x_fp16x16_div(bottom->x - top->x, bottom->y - top->y);
        
        x_fp16x16 topY = x_fp16x16_ceil(top->y);
        x_fp16x16 errorCorrection = x_fp16x16_mul(top->y - topY, xSlope);
        
        x = top->x - errorCorrection;
    }
    
    void emitCachedEdge(X_AE_Surface* surface)
    {
        if(surfaces[X_AE_EDGE_LEFT_SURFACE] == NULL)
            surfaces[X_AE_EDGE_LEFT_SURFACE] = surface;
        else if(surfaces[X_AE_EDGE_RIGHT_SURFACE] == NULL)
            surfaces[X_AE_EDGE_RIGHT_SURFACE] = surface;
        else
        {
            // Should never happen
            printf("Trying to emit full edge\n");
        }
    }
    
    // Attributes shared with X_AE_DummyEdge (do not reorder!)
    x_fp16x16 x;
    struct X_AE_Edge* next;
    
    // Unique to X_AE_Edge
    struct X_AE_Edge* prev;
    
    x_fp16x16 xSlope;
    X_AE_Surface* surfaces[2];
    bool isLeadingEdge;
    bool isHorizontal;
    
    X_BspEdge* bspEdge;
    int frameCreated;
    
    struct X_AE_Edge* nextDelete;
    
    short startY;
    short endY;
} X_AE_Edge;

typedef struct X_AE_DummyEdge
{
    // Attributes shared with X_AE_Edge (do not reorder!)
    x_fp16x16 x;
    struct X_AE_Edge* next;
    
    struct X_AE_Edge* deleteHead;
} X_AE_DummyEdge;

#define X_ACTIVE_SURFACES_SIZE 32

struct X_AE_Context
{
    X_AE_Context(int maxEdges, int maxSurfaces, int maxSpans, X_Screen* screen_)
        : edges(maxEdges, "EdgeArena"),
        surfaces(maxSurfaces, "SurfaceArena"),
        spans(maxSpans, "SpanArena"),
        screen(screen_)
    {
        initSentinalEdges();
        initEdges();
        initSurfaces();
    }
    
    ~X_AE_Context()
    {
        x_free(newEdges);
    }
    
    ArenaAllocator<X_AE_Edge> edges;
    ArenaAllocator<X_AE_Surface> surfaces;
    ArenaAllocator<X_AE_Span> spans;
    
    X_AE_DummyEdge* newEdges;
    X_AE_Edge newRightEdge;
    
    X_AE_Edge leftEdge;
    X_AE_Edge rightEdge;
    
    X_AE_Surface background;
    X_BspSurface backgroundBspSurface;
    
    X_AE_Surface foreground;
    
    X_RenderContext* renderContext;
    X_Screen* screen;
    
    X_BspModel* currentModel;
    X_AE_Surface* currentParent;
    
private:
    void initSentinalEdges()
    {
        leftEdge.x = -0x7FFFFFFF;
        leftEdge.xSlope = 0;
        leftEdge.next = &rightEdge;
        
        rightEdge.x = 0x7FFFFFFF;
        rightEdge.xSlope = 0;
        rightEdge.prev = &leftEdge;
        rightEdge.next = NULL;
    }
    
    void initEdges()
    {
        for(X_AE_Edge* edge = edges.begin(); edge != edges.allocationEnd(); ++edge)
        {
            edge->bspEdge = NULL;
            edge->frameCreated = -1;
        }
        
        // FIXME: screen is not getting initalized before calling this;
        
        //newEdges = (X_AE_DummyEdge*)x_malloc(x_screen_h(screen) * sizeof(X_AE_DummyEdge));
        newEdges = (X_AE_DummyEdge*)x_malloc(640 * sizeof(X_AE_DummyEdge));
    }
    
    void initSurfaces()
    {
        for(X_AE_Surface* surface = surfaces.begin(); surface != surfaces.allocationEnd(); ++surface)
            surface->id = surface - surfaces.begin();
    }
    
    void resetActiveEdges()
    {
        leftEdge.next = &rightEdge;
        leftEdge.prev = NULL;
        
        rightEdge.prev = &leftEdge;
        rightEdge.next = &newRightEdge;
    }
    
    void resetBackgroundSurface()
    {
        leftEdge.surfaces[X_AE_EDGE_RIGHT_SURFACE] = &background;
        leftEdge.surfaces[X_AE_EDGE_LEFT_SURFACE] = NULL;
        
        rightEdge.surfaces[X_AE_EDGE_LEFT_SURFACE] = &background;
        rightEdge.surfaces[X_AE_EDGE_RIGHT_SURFACE] = NULL;
        
        foreground.next = &background;
        foreground.prev = NULL;
        foreground.bspKey = -0x7FFFFFFF;
        foreground.bspSurface = &backgroundBspSurface;
        foreground.parent = &foreground;
        foreground.last = &foreground.spanHead;
        
        background.next = NULL;
        background.prev = &foreground;
        background.bspKey = 0x7FFFFFFF;
        background.bspSurface = &backgroundBspSurface;
        background.parent = &background;
        background.last = &background.spanHead;
    }
    
    void resetArenas()
    {
        edges.freeAll();
        spans.freeAll();
        surfaces.freeAll();
    }
    
    void resetNewEdges()
    {
        newRightEdge.x = rightEdge.x;
        newRightEdge.next = &newRightEdge;    // Loop back around
        
        // TODO: we can probably reset these as we sort in the new edges for each scanline
        for(int i = 0; i < x_screen_h(screen); ++i)
        {
            newEdges[i].next = (X_AE_Edge*)&newRightEdge;
            newEdges[i].x = x_fp16x16_from_float(-1000);
            newEdges[i].deleteHead = NULL;
        }
    }
    
    bool addEdgeToStartingScanline(X_AE_Edge* newEdge)
    {
        X_AE_Edge* edge = (X_AE_Edge*)&newEdges[newEdge->startY];
        
        // FIXME: why is this check here?
        if(!edge)
            return false;
        
        if(newEdge->endY < 0)
            return false;
        
        newEdge->nextDelete = newEdges[newEdge->endY].deleteHead;
        newEdges[newEdge->endY].deleteHead = newEdge;
        
        x_fp16x16 edgeX = newEdge->x;
        
        if(newEdge->surfaces[X_AE_EDGE_RIGHT_SURFACE] != NULL)
            ++edgeX;
        
        // TODO: the edges should be moved into an array in sorted - doing it a linked list is O(n^2)
        while(edge->next->x < edgeX)
        {
            edge = edge->next;
        }
        
        newEdge->next = edge->next;
        edge->next = newEdge;
        
        return true;
    }
    
    void addEdge(X_Vec2_fp16x16* a, X_Vec2_fp16x16* b, X_AE_Surface* surface, X_BspEdge* bspEdge)
    {
        X_AE_Edge* edge = edges.alloc();
        
        new (edge) X_AE_Edge(a, b, surface, bspEdge, renderContext->currentFrame, edges.begin());
        
        if(edge->isHorizontal)
        {
            edges.freeLast();
        }
        else
        {
            addEdgeToStartingScanline(edge);
        }
    }
    
    // FIXME
    friend void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext);
    friend void x_ae_context_scan_edges(X_AE_Context* context);
    friend void emit_edges(X_AE_Context* context, X_AE_Surface* surface, X_Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds);
};


void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext);

void x_ae_context_add_level_polygon(X_AE_Context* context,
                                    X_BspLevel* level,
                                    int* edgeIds,
                                    int totalEdges,
                                    X_BspSurface* bspSurface,
                                    BoundBoxFrustumFlags geoFlags,
                                    int bspKey
                                   );

void x_ae_context_add_submodel_polygon(X_AE_Context* context, X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey);


void x_ae_context_scan_edges(X_AE_Context* context);

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, X_BspLevel* level);

static inline x_fp16x16 x_ae_surface_calculate_inverse_z_at_screen_point(const X_AE_Surface* surface, int x, int y)
{
    return x * surface->zInverseXStep + y * surface->zInverseYStep + surface->zInverseOrigin;
}

static inline void x_ae_context_set_current_model(X_AE_Context* context, X_BspModel* model)
{
    context->currentModel = model;
}

static inline void x_ae_surface_reset_current_parent(X_AE_Context* context)
{
    context->currentParent = NULL;
}

