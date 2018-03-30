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
#include "geo/X_Polygon2.hpp"
#include "X_span.h"
#include "render/X_Renderer.h"
#include "engine/X_EngineContext.h"
#include "object/X_CameraObject.h"

int g_sortCount;
int g_stackCount;

void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext)
{
    g_sortCount = 0;
    g_stackCount = 0;

    context->renderContext = renderContext;
    
    context->resetActiveEdges();
    context->resetArenas();
    context->resetNewEdges();
}

static bool edge_is_flipped(int edgeId)
{
    return edgeId < 0;
}

static X_AE_Edge* get_cached_edge(X_AE_Context* context, X_BspEdge* edge, int currentFrame)
{
    X_AE_Edge* aeEdge = (X_AE_Edge*)((unsigned char*)context->edges.begin() + edge->cachedEdgeOffset);

    if(aeEdge->frameCreated != currentFrame || aeEdge->bspEdge != edge)
        return NULL;

    return aeEdge;
}

// TODO: no need to project vertices if can just reuse the cached edge
static bool project_polygon3(Polygon3* poly, X_Mat4x4* viewMatrix, X_Viewport* viewport, Polygon2* dest, x_fp16x16* closestZ)
{
    *closestZ = 0x7FFFFFFF;
    
    for(int i = 0; i < poly->totalVertices; ++i)
    {
        Vec3 transformed;
        x_mat4x4_transform_vec3(viewMatrix, poly->vertices + i, &transformed);
        
        if(transformed.z < x_fp16x16_from_float(16.0))
            transformed.z = x_fp16x16_from_float(16.0);
        
        poly->vertices[i] = transformed;
        
        *closestZ = X_MIN(*closestZ, transformed.z);
        
        x_viewport_project_vec3(viewport, &transformed, dest->vertices + i);
        x_viewport_clamp_vec2_fp16x16(viewport, dest->vertices + i);
    }
    
    dest->totalVertices = poly->totalVertices;

    return 1;
}

static X_AE_Surface* create_ae_surface(X_AE_Context* context, X_BspSurface* bspSurface, int bspKey)
{
    X_AE_Surface* surface = context->surfaces.alloc();

    surface->last = &surface->spanHead;
    surface->bspKey = bspKey;
    surface->bspSurface = bspSurface;
    surface->crossCount = 0;
    surface->closestZ = 0x7FFFFFFF;
    surface->modelOrigin = &context->currentModel->origin;
    
    if(context->currentParent == NULL)
        context->currentParent = surface;
    
    surface->parent = context->currentParent;

    return surface;
}

void emit_edges(X_AE_Context* context, X_AE_Surface* surface, X_Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds)
{
    for(int i = 0; i < totalVertices; ++i)
    {
        int edgeId = abs(clippedEdgeIds[i]);
        X_BspEdge* bspEdge = context->renderContext->level->edges + edgeId;

        if(edgeId != 0)
        {
            X_AE_Edge* cachedEdge = get_cached_edge(context, bspEdge, context->renderContext->currentFrame);

            if(cachedEdge != NULL)
            {
                cachedEdge->emitCachedEdge(surface);
                continue;
            }
        }

        int next = (i + 1 < totalVertices ? i + 1 : 0);
        context->addEdge(v2d + i, v2d + next, surface, bspEdge);
    }
}

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, X_BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, x_fp16x16* closestZ)
{
    Vec3 clippedV[X_POLYGON3_MAX_VERTS];
    LevelPolygon3 clipped(clippedV, X_POLYGON3_MAX_VERTS, dest->edgeIds);
    
    if(clipFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
    {
        clipped.vertices = poly->vertices;
        clipped.totalVertices = poly->totalVertices;
        clipped.edgeIds = poly->edgeIds;
    }
    else if(!poly->clipToFrustumPreserveEdgeIds(*renderContext->viewFrustum, clipped, clipFlags, poly->edgeIds, dest->edgeIds))
    {
        return false;
    }
    
    return project_polygon3(&clipped, renderContext->viewMatrix, &renderContext->cam->viewport, dest, closestZ);
}

// TODO: check whether edgeIds is NULL
void x_ae_context_add_polygon(X_AE_Context* context, Polygon3* polygon, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int* edgeIds, int bspKey, bool inSubmodel)
{
    Vec3 firstVertex = polygon->vertices[0];
    
    ++context->renderContext->renderer->totalSurfacesRendered;

    int clippedEdgeIds[X_POLYGON3_MAX_VERTS] = { 0 };
    
    LevelPolygon3 levelPoly(polygon->vertices, polygon->totalVertices, edgeIds);

    X_Vec2_fp16x16 v2d[X_POLYGON3_MAX_VERTS];
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    
    x_fp16x16 closestZ;
    if(!projectAndClipBspPolygon(&levelPoly, context->renderContext, geoFlags, &poly2d, &closestZ))
        return;
    
    X_AE_Surface* surface = create_ae_surface(context, bspSurface, bspKey);

    surface->inSubmodel = inSubmodel;
    surface->closestZ = closestZ;

    // FIXME: shouldn't be done this way
    Vec3 camPos = x_cameraobject_get_position(context->renderContext->cam);
    
    surface->calculateInverseZGradient(&camPos, &context->renderContext->cam->viewport, context->renderContext->viewMatrix, &firstVertex);
    emit_edges(context, surface, v2d, poly2d.totalVertices, poly2d.edgeIds);
}

static void get_level_polygon_from_edges(X_BspLevel* level, int* edgeIds, int totalEdges, Polygon3* dest, Vec3* origin)
{
    dest->totalVertices = 0;
    
    for(int i = 0; i < totalEdges; ++i)
    {
        Vec3 v;
        
        if(!edge_is_flipped(edgeIds[i]))
            v = level->vertices[level->edges[edgeIds[i]].v[0]].v;
        else
            v = level->vertices[level->edges[-edgeIds[i]].v[1]].v;
                
        dest->vertices[dest->totalVertices++] = v + *origin;
    }
}

void x_ae_context_add_level_polygon(X_AE_Context* context, X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(context);
    
    InternalPolygon3 polygon;

    // FIXME: why is this here?
    polygon.totalVertices = 0;

    get_level_polygon_from_edges(level, edgeIds, totalEdges, &polygon, &context->currentModel->origin);

    x_ae_context_add_polygon(context, &polygon, bspSurface, geoFlags, edgeIds, bspKey, 0);
}

static void x_ae_context_add_submodel_polygon_recursive(X_AE_Context* context, Polygon3* poly, X_BspNode* node, int* edgeIds, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
{
    if(!x_bspnode_is_visible_this_frame(node, context->renderContext->currentFrame))
        return;
    
    if(poly->totalVertices < 3)
        return;
    
    if(x_bspnode_is_leaf(node))
    {
        if(node->contents == X_BSPLEAF_SOLID)
            return;
        
        X_BspLeaf* leaf = (X_BspLeaf*)node;
        x_ae_context_add_polygon(context, poly, bspSurface, geoFlags, edgeIds, leaf->bspKey, 1);
        return;
    }
    
    int frontEdges[32] = { 0 };
    int backEdges[32] = { 0 };
    
    InternalPolygon3 front;
    InternalPolygon3 back;
    
    poly->splitAlongPlane(node->plane->plane, edgeIds, front, frontEdges, back, backEdges);
    
    x_ae_context_add_submodel_polygon_recursive(context, &front, node->frontChild, frontEdges, bspSurface, geoFlags, bspKey);
    x_ae_context_add_submodel_polygon_recursive(context, &back, node->backChild, backEdges, bspSurface, geoFlags, bspKey);
}

void x_ae_context_add_submodel_polygon(X_AE_Context* context, X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(context);
    
    InternalPolygon3 poly;
    get_level_polygon_from_edges(level, edgeIds, totalEdges, &poly, &context->currentModel->origin);
    
    x_ae_context_add_submodel_polygon_recursive(context, &poly, x_bsplevel_get_root_node(level), edgeIds, bspSurface, geoFlags, bspKey);
}

static void x_ae_context_emit_span(X_AE_Context* context, int left, int right, int y, X_AE_Surface* surface)
{
    if(left == right)
        return;
    
    surface = surface->parent;

    X_AE_Span* span = context->spans.alloc();
    
    span->x1 = left;
    span->x2 = right;
    span->y = y;
    
    surface->last->next = span;
    surface->last = span;
}

static void remove_from_surface_stack(X_AE_Surface* surface)
{
    surface->prev->next = surface->next;
    surface->next->prev = surface->prev;
}

// void validate_stack(X_AE_Context* c, const char* msg)
// {
//     return;
//     printf("=============%s\n", msg);
//     
//     x_assert(c->foreground.prev == NULL, "Bad f prev");
//     x_assert(c->background.next == NULL, "Bad b prev");
//     
//     X_AE_Surface* prev = NULL;
//     
//     for(X_AE_Surface* s = &c->foreground; s != NULL; s = s->next)
//     {
//         //printf("Check %d\n", s->id);
//         
//         x_assert(s->prev == prev, "Bad prev");
//         prev = s;
//     }
// }

static inline void x_ae_context_process_edge(X_AE_Context* context, X_AE_Edge* edge, int y)
{
    X_AE_Surface* surfaceToEnable = edge->surfaces[X_AE_EDGE_RIGHT_SURFACE];
    X_AE_Surface* surfaceToDisable = edge->surfaces[X_AE_EDGE_LEFT_SURFACE];

    X_AE_Surface* topSurface = context->foreground.next;
    
    if(surfaceToDisable != NULL)
    {
        // Make sure the edges didn't cross
        if(--surfaceToDisable->crossCount != 0)
        {
            surfaceToDisable = NULL;
            goto enable;
        }

        
        if(surfaceToDisable == topSurface)
        {
            // We were on top, so emit the span
            int x = (edge->x) >> 16;

            x_ae_context_emit_span(context, surfaceToDisable->xStart, x, y, surfaceToDisable);
            
            topSurface->next->xStart = x;
        }
        
        remove_from_surface_stack(surfaceToDisable);
    }

enable:
    if(surfaceToEnable != NULL)
    {
        topSurface = context->foreground.next;
        
        // Make sure the edges didn't cross
        if(++surfaceToEnable->crossCount != 1)
            return;
        
        X_AE_Surface* search;
        if(surfaceToDisable != NULL)
            search = surfaceToDisable->prev;
        else
            search = topSurface;
        
        if(surfaceToEnable->isCloserThan(search, edge->x, y))
        {
            do
            {
                search = search->prev;
            } while(surfaceToEnable->isCloserThan(search, edge->x, y));
            
            
            search = search->next;
        }
        else
        {
            do
            {
                search = search->next;
            } while(!surfaceToEnable->isCloserThan(search, edge->x, y));            
        }
        
        surfaceToEnable->next = search;
        surfaceToEnable->prev = search->prev;
        search->prev->next = surfaceToEnable;
        search->prev = surfaceToEnable;

        // Are we the top surface now?
        if(context->foreground.next == surfaceToEnable)
        {
            // Yes, emit span for the old top
            int x = edge->x >> 16;

            x_ae_context_emit_span(context, topSurface->xStart, x, y, topSurface);

            surfaceToEnable->xStart = x;
            //context->foreground.next = surfaceToEnable;
        }        
    }
}

static inline void x_ae_context_add_active_edge(X_AE_Context* context, X_AE_Edge* edge, int y)
{
    x_ae_context_process_edge(context, edge, y);

    // Advance the edge
    edge->x += edge->xSlope;
    
    while(edge->x < edge->prev->x)
    {
        X_AE_Edge* prev = edge->prev;
        prev->next = edge->next;
        edge->next->prev = prev;

        prev->prev->next = edge;
        edge->prev = prev->prev;

        edge->next = prev;
        prev->prev = edge;
    }
}

static inline void x_ae_context_process_edges(X_AE_Context* context, int y)
{
    context->background.crossCount = 1;
    context->background.xStart = 0;
    context->foreground.next = &context->background;
    context->foreground.prev = NULL;
    
    context->background.next = NULL;
    context->background.prev = &context->foreground;

    X_AE_Edge* activeEdge = context->leftEdge.next;

    X_AE_Edge* newEdge = context->newEdges[y].next;

    
    while(newEdge != &context->newRightEdge)
    {
        while(activeEdge->x <= newEdge->x)
            activeEdge = activeEdge->next;
        
        X_AE_Edge* newEdgeNext = newEdge->next;
        X_AE_Edge* prev = activeEdge->prev;
        prev->next = newEdge;
        newEdge->prev = prev;
        
        newEdge->next = activeEdge;
        activeEdge->prev = newEdge;
        
        newEdge = newEdgeNext;
    }
    

    for(X_AE_Edge* edge = context->leftEdge.next; edge != &context->rightEdge; )
    {
        X_AE_Edge* next = edge->next;
        x_ae_context_add_active_edge(context, edge, y);
        edge = next;
    }


    X_AE_Edge* nextDelete = context->newEdges[y].deleteHead;

    while(nextDelete)
    {
        nextDelete->prev->next = nextDelete->next;
        nextDelete->next->prev = nextDelete->prev;

        nextDelete = nextDelete->nextDelete;
    }
}

void __attribute__((hot)) x_ae_context_scan_edges(X_AE_Context* context)
{
    static bool initialized = 0;

    if(!initialized)
    {
        x_console_register_var(&context->renderContext->engineContext->console, &g_sortCount, "sortCount", X_CONSOLEVAR_INT, "0", 0);
        x_console_register_var(&context->renderContext->engineContext->console, &g_stackCount, "stackCount", X_CONSOLEVAR_INT, "0", 0);
        initialized = 1;
    }

    context->resetBackgroundSurface();

    // Don't bother removing the edges from the last scanline
    context->newEdges[x_screen_h(context->screen) - 1].deleteHead = NULL;
    
    if((context->renderContext->renderer->renderMode & 2) != 0)
    {
        for(int i = 0; i < x_screen_h(context->screen); ++i)
        {
            x_ae_context_process_edges(context, i);
        }
    }

    if((context->renderContext->renderer->renderMode & 1) == 0)
        return;

    for(X_AE_Surface* surface = context->surfaces.begin(); surface != context->surfaces.end(); ++surface)
    {
        if(surface->last == &surface->spanHead)
            continue;

        X_AE_SurfaceRenderContext surfaceRenderContext;
        x_ae_surfacerendercontext_init(&surfaceRenderContext, surface, context->renderContext, context->renderContext->renderer->mipLevel);
        x_ae_surfacerendercontext_render_spans(&surfaceRenderContext);
    }
}

bool x_ae_surface_point_is_in_surface_spans(X_AE_Surface* surface, int x, int y)
{
    X_AE_Span* span = surface->spanHead.next;
    
    while(span)
    {
        if(span->y == y && x >= span->x1 && x < span->x2)
            return true;
        
        span = span->next;
    }

    return false;
}

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, X_BspLevel* level)
{
    for(X_AE_Surface* s = context->surfaces.begin(); s != context->surfaces.end(); ++s)
    {
        if(x_ae_surface_point_is_in_surface_spans(s, x, y))
            return s->bspSurface - level->surfaces;
    }

    return -1;
}
