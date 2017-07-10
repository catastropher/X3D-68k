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

#include "error/X_error.h"
#include "memory/X_alloc.h"
#include "util/X_util.h"
#include "X_activeedge.h"
#include "level/X_BspLevel.h"

static void x_ae_context_init_sentinal_edges(X_AE_Context* context)
{
    context->leftEdge.x = x_fp16x16_from_float(-.5);
    context->leftEdge.xSlope = 0;
    context->leftEdge.surface = &context->background;
    
    context->rightEdge.x = x_fp16x16_from_int(x_screen_w(context->screen));
    context->rightEdge.endY = x_screen_h(context->screen);
    context->rightEdge.xSlope = 0;
    context->rightEdge.surface = &context->background;
    context->rightEdge.next = NULL;
}

static void x_ae_context_init_edges(X_AE_Context* context, int maxActiveEdges, int edgePoolSize)
{
    context->maxActiveEdges = maxActiveEdges;
    
    context->activeEdges = x_malloc(maxActiveEdges * sizeof(X_AE_Edge*));
    context->oldActiveEdges = x_malloc(maxActiveEdges * sizeof(X_AE_Edge*));
    
    context->edgePool = x_malloc(edgePoolSize * sizeof(X_AE_Edge));
    context->edgePoolEnd = context->edgePool + edgePoolSize;
    
    context->newEdges = x_malloc(x_screen_h(context->screen) * sizeof(X_AE_DummyEdge));
}

static void x_ae_context_init_surfaces(X_AE_Context* context, int surfacePoolSize)
{
    context->surfacePool = x_malloc(surfacePoolSize * sizeof(X_AE_Surface));
    context->surfacePoolEnd = context->surfacePool + surfacePoolSize;
}

void x_ae_context_init(X_AE_Context* context, X_Screen* screen, int maxActiveEdges, int edgePoolSize, int surfacePoolSize)
{
    context->screen = screen;
    
    x_ae_context_init_sentinal_edges(context);
    x_ae_context_init_edges(context, maxActiveEdges, edgePoolSize);
    x_ae_context_init_surfaces(context, surfacePoolSize);
    x_ae_context_reset(context);
}

void x_ae_context_reset(X_AE_Context* context)
{
    context->totalActiveEdges = 0;
    context->oldTotalActiveEdges = 0;
    
    context->nextAvailableEdge = context->edgePool;
    context->nextAvailableSurface = context->surfacePool;
    
    for(int i = 0; i < x_screen_h(context->screen); ++i)
    {
        context->newEdges[i].next = &context->rightEdge;
        context->newEdges[i].x = x_fp16x16_from_float(-0.5);
    }
    
    context->activeEdges[0] = &context->leftEdge;
    context->activeEdges[1] = &context->rightEdge;
    context->totalActiveEdges = 2;
}

X_AE_Edge* x_ae_context_add_edge(X_AE_Context* context, X_Vec2* a, X_Vec2* b, X_AE_Surface* surface)
{
    int height = b->y - a->y;
    if(height == 0)
        return NULL;
    
    x_assert(context->nextAvailableEdge < context->edgePoolEnd, "AE out of edges");
    
    X_AE_Edge* edge = context->nextAvailableEdge++;
    edge->isLeadingEdge = height > 0;
    
    if(!edge->isLeadingEdge)
        X_SWAP(a, b);
    
    edge->x = x_fp16x16_from_int(a->x) + x_fp16x16_from_float(0.5);
    edge->xSlope = x_int_div_as_fp16x16(b->x - a->x, b->y - a->y);
    edge->endY = b->y - 1;
    edge->surface = surface;
    
    return edge;
}

static _Bool edge_is_flipped(int edgeId)
{
    return edgeId < 0;
}

void x_ae_context_add_level_polygon(X_AE_Context* context, X_BspLevel* level, const int* edgeIds, int totalEdges, X_BspSurface* bspSurface)
{
    x_assert(context->nextAvailableSurface < context->surfacePoolEnd, "AE out of surfaces");
    
    X_AE_Surface* surface = context->nextAvailableSurface++;
    
    surface->bspSurface = bspSurface;
    surface->crossCount = 0;
    
    for(int i = 0; i < totalEdges; ++i)
    {
        X_Vec2* a;
        X_Vec2* b;
        
        if(!edge_is_flipped(edgeIds[i]))
        {
            X_BspEdge* levelEdge = level->edges + edgeIds[i];
            //a = &level->vertices[levelEdge->v[0]].v;
        }
    }
}

