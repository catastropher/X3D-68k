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

#include "math/FixedPoint.hpp"
#include "render/Texture.hpp"
#include "render/RenderContext.hpp"
#include "Screen.hpp"
#include "level/BspLevel.hpp"
#include "Span.hpp"
#include "Viewport.hpp"
#include "geo/Ray3.hpp"
#include "memory/BitSet.hpp"

#include "memory/ArenaAllocator.hpp"

#define X_AE_SURFACE_MAX_SPANS 332

enum SurfaceFlags
{
    SURFACE_NO_DRAW_SPANS = 1,
    SURFACE_FILL_SOLID = 2
};

struct X_AE_Surface
{
    fp inverseZAtScreenPoint(fp x, int y) const
    {
        return x * zInverseXStep + y * zInverseYStep + zInverseOrigin;
    }
    
    bool isCloserThan(const X_AE_Surface* surface, fp x, int y) const
    {
        if(bspKey != surface->bspKey)
        {
            return bspKey < surface->bspKey;
        }
        
        if(inSubmodel ^ surface->inSubmodel)
        {
            return inSubmodel;
        }
        
        return inverseZAtScreenPoint(x + 7.0_fp, y) >= surface->inverseZAtScreenPoint(x + 7.0_fp, y);
    }
    
    void calculateInverseZGradient(Vec3fp& camPos, Viewport* viewport, Mat4x4* viewMatrix, Vec3fp& pointOnSurface)
    {
        Plane planeInViewSpace;
        bspSurface->calculatePlaneInViewSpace(camPos, viewMatrix, pointOnSurface, &planeInViewSpace);
        
        int dist = -planeInViewSpace.d.internalValue();
        int scale = viewport->distToNearPlane;
        
        if(dist == 0 || scale == 0) return;
        
        //x_fp16x16 invDistTimesScale = //x_fp16x16_div(X_FP16x16_ONE << 10, distTimesScale) >> 6;
        
        fp invDist = 1024.0_fp / fp(dist);
        fp invScale = fp((1 << 26) / scale);
        
        fp invDistTimesScale = (invDist * invScale) >> 10;
        
        zInverseXStep = invDistTimesScale * planeInViewSpace.normal.x;
        zInverseYStep = invDistTimesScale * planeInViewSpace.normal.y;
        
        int centerX = viewport->screenPos.x + viewport->w / 2;
        int centerY = viewport->screenPos.y + viewport->h / 2;
        
        zInverseOrigin = planeInViewSpace.normal.z * invDist -
            centerX * zInverseXStep -
            centerY * zInverseYStep;
    }

    void removeFromSurfaceStack()
    {
        prev->next = next;
        next->prev = prev;
    }

    void enableSolidFill(X_Color color)
    {
        flags.set(SURFACE_FILL_SOLID);

        // Top 8 bits are color
        flags.set(color << 24);
    }

    X_Color getSolidFillColor()
    {
        return flags.getMask() >> 24;
    }
    
    int bspKey;
    int id;
    int crossCount;
    int xStart;
    
    BspSurface* bspSurface;
    X_AE_Span spanHead;
    X_AE_Span* last;
    
    fp zInverseXStep;
    fp zInverseYStep;
    fp zInverseOrigin;
    
    fp closestZ;
    
    bool inSubmodel;
    
    Vec3fp* modelOrigin;
    
    struct X_AE_Surface* next;
    struct X_AE_Surface* prev;
    
    struct X_AE_Surface* parent;    

    Flags<SurfaceFlags> flags;
};

#define X_AE_EDGE_LEFT_SURFACE 0
#define X_AE_EDGE_RIGHT_SURFACE 1

typedef struct X_AE_Edge
{
    X_AE_Edge() { }
    
    X_AE_Edge(Vec2_fp16x16* a, Vec2_fp16x16* b, X_AE_Surface* surface, BspEdge* bspEdge_, int currentFrame, X_AE_Edge* edgeStart)
    {
        int aY = fp(a->y).ceil().toInt();
        int bY = fp(b->y).ceil().toInt();
        
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
        
        initDeltas(MakeVec2fp(*a), MakeVec2fp(*b));
        
        startY = x_fp16x16_to_int(fp(a->y).ceil().toFp16x16());
        endY = x_fp16x16_to_int(fp(b->y).ceil().toFp16x16()) - 1;
    }
    
    void initDeltas(const Vec2fp& top, const Vec2fp& bottom)
    {
        xSlope = (bottom.x - top.x) / (bottom.y - top.y);
        
        fp topY = top.y.ceil();
        fp errorCorrection = (top.y - topY) * xSlope;
        
        x = top.x - errorCorrection;
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
    fp x;
    struct X_AE_Edge* next;
    
    // Unique to X_AE_Edge
    struct X_AE_Edge* prev;
    
    fp xSlope;
    X_AE_Surface* surfaces[2];
    bool isLeadingEdge;
    bool isHorizontal;
    
    BspEdge* bspEdge;
    int frameCreated;
    
    struct X_AE_Edge* nextDelete;
    
    short startY;
    short endY;
} X_AE_Edge;

typedef struct X_AE_DummyEdge
{
    // Attributes shared with X_AE_Edge (do not reorder!)
    fp x;
    struct X_AE_Edge* next;
    
    struct X_AE_Edge* deleteHead;
} X_AE_DummyEdge;

#define X_ACTIVE_SURFACES_SIZE 32

struct X_AE_Context
{
    X_AE_Context(int maxEdges, int maxSurfaces, int maxSpans, Screen* screen_)
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
    BspSurface backgroundBspSurface;
    
    X_AE_Surface foreground;
    
    X_RenderContext* renderContext;
    Screen* screen;
    
    BspModel* currentModel;
    X_AE_Surface* currentParent;

    void addSubmodelPolygon(BspLevel* level, int* edgeIds, int totalEdges, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey);
    void addLevelPolygon(BspLevel* level, int* edgeIds, int totalEdges, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey);

    // !-- To be made private --!
    X_AE_Edge* getCachedEdge(BspEdge* edge, int currentFrame) const;
    X_AE_Surface* createSurface(BspSurface* bspSurface, int bspKey);
    void emitEdges(X_AE_Surface* surface, Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds);
    void addPolygon(Polygon3* polygon, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int* edgeIds, int bspKey, bool inSubmodel);
    void addSubmodelRecursive(Polygon3* poly, BspNode* node, int* edgeIds, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey);
    void emitSpan(int left, int right, int y, X_AE_Surface* surface);

    void processEdge(X_AE_Edge* edge, int y);
    void addActiveEdge(X_AE_Edge* edge, int y);
    void processEdges(int y);

    void processPolygon(BspSurface* bspSurface,
                        BoundBoxFrustumFlags geoFlags,
                        int* edgeIds,
                        int bspKey,
                        bool inSubmodel);

    X_AE_Surface* addBrushPolygon(Polygon3& polygon, Plane& polygonPlane, BoundBoxFrustumFlags geoFlags, int bspKey);

    X_AE_Edge* addEdgeFromClippedRay(Ray3& clipped, X_AE_Surface* aeSurface, BspEdge* bspEdge, bool lastWasClipped, Vec2& lastProjected);

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
        for(int i = 0; i < screen->getH(); ++i)
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
        
        fp edgeX = newEdge->x;
        
        if(newEdge->surfaces[X_AE_EDGE_RIGHT_SURFACE] != NULL)
        {
            edgeX += fp(1);
        }
        
        // TODO: the edges should be moved into an array in sorted - doing it a linked list is O(n^2)
        while(edge->next->x < edgeX)
        {
            edge = edge->next;
        }
        
        newEdge->next = edge->next;
        edge->next = newEdge;
        
        return true;
    }
    
    X_AE_Edge* addEdge(Vec2_fp16x16* a, Vec2_fp16x16* b, X_AE_Surface* surface, BspEdge* bspEdge)
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

        return edge;
    }
    
    // FIXME
    friend void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext);
    friend void x_ae_context_scan_edges(X_AE_Context* context);
};


void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext);


void x_ae_context_scan_edges(X_AE_Context* context);

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, BspLevel* level);

static inline fp x_ae_surface_calculate_inverse_z_at_screen_point(const X_AE_Surface* surface, int x, int y)
{
    return x * surface->zInverseXStep + y * surface->zInverseYStep + surface->zInverseOrigin;
}

static inline void x_ae_context_set_current_model(X_AE_Context* context, BspModel* model)
{
    context->currentModel = model;
}

static inline void x_ae_surface_reset_current_parent(X_AE_Context* context)
{
    context->currentParent = NULL;
}

