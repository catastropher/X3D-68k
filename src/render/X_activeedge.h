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

#include "math/X_fix.h"
#include "render/X_Texture.h"
#include "render/X_RenderContext.h"
#include "X_Screen.h"
#include "level/X_BspLevel.h"
#include "X_span.h"

#define X_AE_SURFACE_MAX_SPANS 1024

typedef struct X_AE_Surface
{
    int bspKey;
    int crossCount;
    int xStart;
    
    X_BspSurface* bspSurface;

    X_AE_Span spans[X_AE_SURFACE_MAX_SPANS];
    int totalSpans;
    
    int zInverseShift;          // Number of places we need to shift down zInverseXStep and zInverseYStep to make them an x_fp0x30
    
    int zInverseXStep;          // Fixed point number with a decimal point that is shifted
    int zInverseYStep;
    x_fp2x30 zInverseOrigin;
    
    x_fp16x16 closestZ;
    
    _Bool wasDeleted;
} X_AE_Surface;

typedef struct X_AE_Edge
{
    // Attributes shared with X_AE_DummyEdge (do not reorder!)
    x_fp16x16 x;
    struct X_AE_Edge* next;
    
    // Unique to X_AE_Edge
    x_fp16x16 xSlope;
    X_AE_Surface* surface;
    int endY;
    _Bool isLeadingEdge;
    
    X_BspEdge* bspEdge;
    int frameCreated;
} X_AE_Edge;

typedef struct X_AE_DummyEdge
{
    // Attributes shared with X_AE_Edge (do not reorder!)
    x_fp16x16 x;
    struct X_AE_Edge* next;
} X_AE_DummyEdge;

#define X_ACTIVE_SURFACES_SIZE 32

typedef struct X_AE_Context
{
    X_AE_Edge* edgePool;
    X_AE_Edge* edgePoolEnd;
    X_AE_Edge* nextAvailableEdge;
    
    X_AE_Surface* surfacePool;
    X_AE_Surface* surfacePoolEnd;
    X_AE_Surface* nextAvailableSurface;
    
    int maxActiveEdges;
    
    X_AE_Edge** activeEdges;
    int totalActiveEdges;
    
    X_AE_Edge** oldActiveEdges;
    int oldTotalActiveEdges;
    
    X_AE_DummyEdge* newEdges;
    
    X_AE_Edge leftEdge;
    X_AE_Edge rightEdge;
    
    X_AE_Surface background;
    X_BspSurface backgroundBspSurface;
    X_AE_Surface* backgroundSurface;
    
    X_RenderContext* renderContext;
    X_Screen* screen;
    
    int maxActiveSurface;
    unsigned int activeSurfaces[X_ACTIVE_SURFACES_SIZE];
    int totalActiveSurfaceGroups;
    
    
    int nextBspKey;
} X_AE_Context;

void x_ae_context_init(X_AE_Context* context, X_Screen* screen, int maxActiveEdges, int edgePoolSize, int surfacePoolSize);
void x_ae_context_cleanup(X_AE_Context* context);

void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext);
X_AE_Edge* x_ae_context_add_edge(X_AE_Context* context, X_Vec2* a, X_Vec2* b, X_AE_Surface* surface);
void x_ae_context_add_level_polygon(X_AE_Context* context, X_BspLevel* level, const int* edgeIds, int totalEdges, X_BspSurface* bspSurface, X_BspBoundBoxFrustumFlags geoFlags);
void x_ae_context_scan_edges(X_AE_Context* context);

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, X_BspLevel* level);

static inline x_fp16x16 x_ae_surface_calculate_inverse_z_at_screen_point(const X_AE_Surface* surface, int x, int y)
{
    x_fp2x30 temp = ((long long)x * surface->zInverseXStep + (long long)y * surface->zInverseYStep) >> surface->zInverseShift;

    return temp + surface->zInverseOrigin;
}
