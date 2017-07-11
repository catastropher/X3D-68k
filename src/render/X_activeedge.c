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
#include "geo/X_Polygon3.h"

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
}

void x_ae_context_reset(X_AE_Context* context, X_RenderContext* renderContext)
{
    context->renderContext = renderContext;
    
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
    
    context->background.bspSurface = &context->backgroundBspSurface;
    context->background.bspKey = 0x7FFFFFFF;
    context->backgroundBspSurface.color = 255;
    
    context->nextBspKey = 0;
}

static void x_ae_context_add_edge_to_starting_scanline(X_AE_Context* context, X_AE_Edge* newEdge, int startY)
{
    X_AE_Edge* edge = (X_AE_Edge*)&context->newEdges[startY];
    
    while(edge->next->x < newEdge->x)
        edge = edge->next;
    
    newEdge->next = edge->next;
    edge->next = newEdge;
}

X_AE_Edge* x_ae_context_add_edge(X_AE_Context* context, X_Vec2* a, X_Vec2* b, X_AE_Surface* surface)
{
    int height = b->y - a->y;
    if(height == 0)
        return NULL;
    
    x_assert(context->nextAvailableEdge < context->edgePoolEnd, "AE out of edges");
    
    X_AE_Edge* edge = context->nextAvailableEdge++;
    edge->isLeadingEdge = height < 0;
    
    if(edge->isLeadingEdge)
        X_SWAP(a, b);
    
    edge->x = x_fp16x16_from_int(a->x) + x_fp16x16_from_float(0.5);
    edge->xSlope = x_int_div_as_fp16x16(b->x - a->x, b->y - a->y);
    edge->endY = b->y - 1;
    edge->surface = surface;
    
    x_ae_context_add_edge_to_starting_scanline(context, edge, a->y);
    
    return edge;
}

static _Bool edge_is_flipped(int edgeId)
{
    return edgeId < 0;
}

#include "geo/X_Ray3.h"

void x_ae_context_add_level_polygon(X_AE_Context* context, X_BspLevel* level, const int* edgeIds, int totalEdges, X_BspSurface* bspSurface)
{
    x_assert(context->nextAvailableSurface < context->surfacePoolEnd, "AE out of surfaces");
    
    X_Vec3 v3d[100];
    X_Polygon3 polygon = x_polygon3_make(v3d, 100);
  
    polygon.totalVertices = 0;
    
    for(int i = 0; i < totalEdges; ++i)
    {
        if(!edge_is_flipped(edgeIds[i]))
            polygon.vertices[polygon.totalVertices++] = level->vertices[level->edges[edgeIds[i]].v[1]].v;
        else
            polygon.vertices[polygon.totalVertices++] = level->vertices[level->edges[-edgeIds[i]].v[0]].v;
    }
    
    X_Vec3 clippedV[100];
    X_Polygon3 clipped = x_polygon3_make(clippedV, 100);
    
    if(!x_polygon3_clip_to_frustum(&polygon, context->renderContext->viewFrustum, &clipped))
    {
        return;
    }
 
    
    X_AE_Surface* surface = context->nextAvailableSurface++;
    
    surface->bspKey = context->nextBspKey++;
    surface->bspSurface = bspSurface;
    surface->crossCount = 0;
    
    X_Vec2 v2d[100];
    for(int i = 0; i < clipped.totalVertices; ++i)
    {
        X_Vec3 transformed;
        x_mat4x4_transform_vec3(context->renderContext->viewMatrix, clipped.vertices + i, &transformed);
        
        x_viewport_project(&context->renderContext->cam->viewport, &transformed, v2d + i);
        x_viewport_clamp_vec2(&context->renderContext->cam->viewport, v2d + i);
    }
    
    for(int i = 0; i < clipped.totalVertices; ++i)
    {
        int next = (i + 1 < clipped.totalVertices ? i + 1 : 0);
        x_ae_context_add_edge(context, v2d + i, v2d + next, surface);
        //x_canvas_draw_line(&context->screen->canvas, v2d[i], v2d[next], 255);
    }
}

static void x_ae_context_emit_span(X_AE_Context* context, int left, int right, int y, X_AE_Surface* surface)
{
    if(right < left)
        X_SWAP(left, right);
    
    memset(context->screen->canvas.tex.texels + x_texture_texel_index(&context->screen->canvas.tex, left, y), surface->bspSurface->color, right - left);
}

static _Bool x_ae_surface_closer(X_AE_Surface* a, X_AE_Surface* b)
{
    return a->bspKey < b->bspKey;
}

static inline void x_ae_context_process_edge(X_AE_Context* context, X_AE_Edge* edge, int y) {
    X_AE_Surface* s = edge->surface;
    X_AE_Surface* currentTop = context->background.next;
    
    if(edge->isLeadingEdge) {        
        // Make sure the edges didn't cross
        if(++s->crossCount != 1)
            return;
        
        // Are we the top surface now?
        if(x_ae_surface_closer(s, currentTop)) {
            // Yes, emit span for the current top
            int x = edge->x >> 16;
            
            x_ae_context_emit_span(context, currentTop->xStart, x, y, currentTop);
            
            s->xStart = x;
            s->next = currentTop;
            s->prev = &context->background;
            
            currentTop->prev = s;
            context->background.next = s;
        }
        else {
            // Sort into the surface stack
            do {
                currentTop = currentTop->next;
            } while(x_ae_surface_closer(currentTop, s));
            
            s->next = currentTop;
            s->prev = currentTop->prev;
            currentTop->prev->next = s;
            currentTop->prev = s;
        }
    }
    else {
        // Make sure the edges didn't cross
        if(--s->crossCount != 0)
            return;
            
        if(s == currentTop) {
            // We were on top, so emit the span
            int x = (edge->x) >> 16;
            
            x_ae_context_emit_span(context, s->xStart, x, y, s);
            
            s->next->xStart = x;
        }
        
        // Remove the surface from the surface stack
        s->next->prev = s->prev;
        s->prev->next = s->next;
    }
}

static inline void x_ae_context_add_active_edge(X_AE_Context* context, X_AE_Edge* edge, int y) {
    x_ae_context_process_edge(context, edge, y);
    
    if(edge->endY == y)
        return;
    
    int pos = context->totalActiveEdges++;
    
    // Advance the edge
    edge->x += edge->xSlope;
    
    // Resort the edge, if it moved out of order
    while(edge->x < context->activeEdges[pos - 1]->x) {
        context->activeEdges[pos] = context->activeEdges[pos - 1];
        --pos;
    }
    
    context->activeEdges[pos] = edge;
}

static inline void x_ae_context_process_edges(X_AE_Context* context, int y) {
    context->background.crossCount = 1;
    context->background.xStart = 0;
    context->background.next = &context->background;
    context->background.prev = &context->background;
    
    X_SWAP(context->oldActiveEdges, context->activeEdges);
    context->oldTotalActiveEdges = context->totalActiveEdges;
    
    context->totalActiveEdges = 1;
    context->activeEdges[0] = &context->leftEdge;
    
    X_AE_Edge** oldActiveEdge = context->oldActiveEdges + 1;
    
    X_AE_Edge* newEdge = context->newEdges[y].next;
    
    while(1) {
        X_AE_Edge* e;
        
        if((*oldActiveEdge)->x < newEdge->x) {
            e = *oldActiveEdge;
            ++oldActiveEdge;
        }
        else {
            e = newEdge;
            newEdge = newEdge->next;
        }
        
        if(e == &context->rightEdge)
            break;
        
        x_ae_context_add_active_edge(context, e, y);
    }
    
    context->activeEdges[context->totalActiveEdges++] = &context->rightEdge;
}

void x_ae_context_scan_edges(X_AE_Context* context)
{
    for(int i = 0; i < x_screen_h(context->screen); ++i)
    {
        for(int j = 0; j < context->nextAvailableSurface - context->surfacePool; ++j)
        {
            context->surfacePool[j].crossCount = 0;
        }
        
        x_ae_context_process_edges(context, i);
    }
}

