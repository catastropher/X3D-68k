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
#include "util/X_StopWatch.hpp"

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

X_AE_Edge* X_AE_Context::getCachedEdge(X_BspEdge* edge, int currentFrame) const
{
    X_AE_Edge* aeEdge = (X_AE_Edge*)((unsigned char*)edges.begin() + edge->cachedEdgeOffset);

    if(aeEdge->frameCreated != currentFrame || aeEdge->bspEdge != edge)
    {
        return NULL;
    }

    return aeEdge;
}

// TODO: no need to project vertices if can just reuse the cached edge
static bool project_polygon3(Polygon3* poly, Mat4x4* viewMatrix, Viewport* viewport, Polygon2* dest, x_fp16x16* closestZ)
{
    *closestZ = 0x7FFFFFFF;
    
    for(int i = 0; i < poly->totalVertices; ++i)
    {
        Vec3 transformed = MakeVec3(viewMatrix->transform(poly->vertices[i]));
        
        const fp minZ = fp::fromFloat(0.5);

        if(transformed.z < minZ.toFp16x16())
            transformed.z = minZ.toFp16x16();
        
        poly->vertices[i] = MakeVec3fp(transformed);
        
        *closestZ = X_MIN(*closestZ, transformed.z);
        
        Vec3fp tempTransformed = MakeVec3fp(transformed);

        viewport->project(tempTransformed, dest->vertices[i]);
        viewport->clampfp(dest->vertices[i]);
    }
    
    dest->totalVertices = poly->totalVertices;

    return 1;
}

X_AE_Surface* X_AE_Context::createSurface(X_BspSurface* bspSurface, int bspKey)
{
    X_AE_Surface* surface = surfaces.alloc();

    surface->last = &surface->spanHead;
    surface->bspKey = bspKey;
    surface->bspSurface = bspSurface;
    surface->crossCount = 0;
    surface->closestZ = 0x7FFFFFFF;
    surface->modelOrigin = &currentModel->origin;
    
    if(currentParent == NULL)
    {
        currentParent = surface;
    }
    
    surface->parent = currentParent;

    return surface;
}

void X_AE_Context::emitEdges(X_AE_Surface* surface, X_Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds)
{
    for(int i = 0; i < totalVertices; ++i)
    {
        int edgeId = abs(clippedEdgeIds[i]);
        X_BspEdge* bspEdge = renderContext->level->edges + edgeId;

        if(edgeId != 0)
        {
            X_AE_Edge* cachedEdge = getCachedEdge(bspEdge, renderContext->currentFrame);

            if(cachedEdge != NULL)
            {
                cachedEdge->emitCachedEdge(surface);
                continue;
            }
        }

        int next = (i + 1 < totalVertices ? i + 1 : 0);
        addEdge(v2d + i, v2d + next, surface, bspEdge);
    }
}

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, x_fp16x16* closestZ)
{
    Vec3 clippedV[X_POLYGON3_MAX_VERTS];
    LevelPolygon3 clipped(clippedV, X_POLYGON3_MAX_VERTS, dest->edgeIds);
    
    if(clipFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
    {
        clipped.vertices = poly->vertices;
        clipped.totalVertices = poly->totalVertices;
        dest->edgeIds = poly->edgeIds;
    }
    else if(!poly->clipToFrustumPreserveEdgeIds(*renderContext->viewFrustum, clipped, clipFlags, poly->edgeIds, dest->edgeIds))
    {
        return false;
    }
    
    return project_polygon3(&clipped, renderContext->viewMatrix, &renderContext->cam->viewport, dest, closestZ);
}

// TODO: check whether edgeIds is NULL
void X_AE_Context::addPolygon(Polygon3* polygon, X_BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int* edgeIds, int bspKey, bool inSubmodel)
{
    Vec3 firstVertex = MakeVec3(polygon->vertices[0]);
    
    ++renderContext->renderer->totalSurfacesRendered;

    int clippedEdgeIds[X_POLYGON3_MAX_VERTS] = { 0 };
    
    LevelPolygon3 levelPoly(polygon->vertices, polygon->totalVertices, edgeIds);

    X_Vec2_fp16x16 v2d[X_POLYGON3_MAX_VERTS];
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    
    x_fp16x16 closestZ;
    if(!projectAndClipBspPolygon(&levelPoly, renderContext, geoFlags, &poly2d, &closestZ))
        return;
    
    X_AE_Surface* surface = createSurface(bspSurface, bspKey);

    surface->inSubmodel = inSubmodel;
    surface->closestZ = closestZ;

    // FIXME: shouldn't be done this way
    Vec3 camPos = x_cameraobject_get_position(renderContext->cam);
    
    surface->calculateInverseZGradient(&camPos, &renderContext->cam->viewport, renderContext->viewMatrix, &firstVertex);
    emitEdges(surface, v2d, poly2d.totalVertices, poly2d.edgeIds);
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
                
        dest->vertices[dest->totalVertices++] = MakeVec3fp(v + *origin);
    }
}

void X_AE_Context::addLevelPolygon(X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(this);
    
    InternalPolygon3 polygon;

    // FIXME: why is this here?
    polygon.totalVertices = 0;

    get_level_polygon_from_edges(level, edgeIds, totalEdges, &polygon, &currentModel->origin);

    addPolygon(&polygon, bspSurface, geoFlags, edgeIds, bspKey, 0);
}

void X_AE_Context::addSubmodelRecursive(Polygon3* poly, X_BspNode* node, int* edgeIds, X_BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    if(!x_bspnode_is_visible_this_frame(node, renderContext->currentFrame))
        return;
    
    if(poly->totalVertices < 3)
        return;
    
    if(x_bspnode_is_leaf(node))
    {
        if(node->contents == X_BSPLEAF_SOLID)
            return;
        
        X_BspLeaf* leaf = (X_BspLeaf*)node;
        addPolygon(poly, bspSurface, geoFlags, edgeIds, leaf->bspKey, true);
        return;
    }
    
    int frontEdges[32] = { 0 };
    int backEdges[32] = { 0 };
    
    InternalPolygon3 front;
    InternalPolygon3 back;
    
    poly->splitAlongPlane(node->plane->plane, edgeIds, front, frontEdges, back, backEdges);
    
    addSubmodelRecursive(&front, node->frontChild, frontEdges, bspSurface, geoFlags, bspKey);
    addSubmodelRecursive(&back, node->backChild, backEdges, bspSurface, geoFlags, bspKey);
}

void X_AE_Context::addSubmodelPolygon(X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(this);
    
    InternalPolygon3 poly;
    get_level_polygon_from_edges(level, edgeIds, totalEdges, &poly, &currentModel->origin);
    
    addSubmodelRecursive(&poly, x_bsplevel_get_root_node(level), edgeIds, bspSurface, geoFlags, bspKey);
}

void X_AE_Context::emitSpan(int left, int right, int y, X_AE_Surface* surface)
{
    if(left == right)
        return;
    
    surface = surface->parent;

    X_AE_Span* span = spans.alloc();
    
    span->x1 = left;
    span->x2 = right;
    span->y = y;
    
    surface->last->next = span;
    surface->last = span;
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

void X_AE_Context::processEdge(X_AE_Edge* edge, int y)
{
    X_AE_Surface* surfaceToEnable = edge->surfaces[X_AE_EDGE_RIGHT_SURFACE];
    X_AE_Surface* surfaceToDisable = edge->surfaces[X_AE_EDGE_LEFT_SURFACE];

    X_AE_Surface* topSurface = foreground.next;
    
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

            emitSpan(surfaceToDisable->xStart, x, y, surfaceToDisable);
            
            topSurface->next->xStart = x;
        }
        
        surfaceToDisable->removeFromSurfaceStack();
    }

enable:
    if(surfaceToEnable != NULL)
    {
        topSurface = foreground.next;
        
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
        if(foreground.next == surfaceToEnable)
        {
            // Yes, emit span for the old top
            int x = edge->x >> 16;

            emitSpan(topSurface->xStart, x, y, topSurface);

            surfaceToEnable->xStart = x;
            //context->foreground.next = surfaceToEnable;
        }        
    }
}

void X_AE_Context::addActiveEdge(X_AE_Edge* edge, int y)
{
    processEdge(edge, y);

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

void X_AE_Context::processEdges(int y)
{
    background.crossCount = 1;
    background.xStart = 0;
    foreground.next = &background;
    foreground.prev = NULL;
    
    background.next = NULL;
    background.prev = &foreground;

    X_AE_Edge* activeEdge = leftEdge.next;

    X_AE_Edge* newEdge = newEdges[y].next;

    
    while(newEdge != &newRightEdge)
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
    

    for(X_AE_Edge* edge = leftEdge.next; edge != &rightEdge; )
    {
        X_AE_Edge* next = edge->next;
        addActiveEdge(edge, y);
        edge = next;
    }


    X_AE_Edge* nextDelete = newEdges[y].deleteHead;

    while(nextDelete)
    {
        nextDelete->prev->next = nextDelete->next;
        nextDelete->next->prev = nextDelete->prev;

        nextDelete = nextDelete->nextDelete;
    }
}

void __attribute__((hot)) x_ae_context_scan_edges(X_AE_Context* context)
{
    // for(X_AE_Edge* edge = context->edges.begin(); edge != context->edges.end(); ++edge)
    // {
    //     if((edge->surfaces[0] == NULL) ^ (edge->surfaces[1] == NULL))
    //     {
    //         x_texture_draw_line(&context->screen->canvas,
    //                             X_Vec2(edge->x >> 16, edge->startY),
    //                             X_Vec2((edge->x + edge->xSlope * (edge->endY - edge->startY)) >> 16, edge->endY),
    //                             context->screen->palette->brightRed);
    //     }
    //     else
    //     {
    //         x_texture_draw_line(&context->screen->canvas,
    //                             X_Vec2(edge->x >> 16, edge->startY),
    //                             X_Vec2((edge->x + edge->xSlope * (edge->endY - edge->startY)) >> 16, edge->endY),
    //                             context->screen->palette->darkGreen);
    //     }
    // }
    
    // return;
    
    static bool initialized = 0;

    if(!initialized)
    {
        x_console_register_var(context->renderContext->engineContext->getConsole(), &g_sortCount, "sortCount", X_CONSOLEVAR_INT, "0", 0);
        x_console_register_var(context->renderContext->engineContext->getConsole(), &g_stackCount, "stackCount", X_CONSOLEVAR_INT, "0", 0);
        initialized = 1;
    }

    context->resetBackgroundSurface();

    // Don't bother removing the edges from the last scanline
    context->newEdges[x_screen_h(context->screen) - 1].deleteHead = NULL;

    StopWatch::start("scan-active-edge");
    
    if((context->renderContext->renderer->renderMode & 2) != 0)
    {
        for(int i = 0; i < context->renderContext->cam->viewport.h; ++i)
        {
            context->processEdges(i);

            // FIXME: temporary fix for disappearing polygons due to crossCount not getting reset
            // on each scanline. Should maybe keep a list of active surfaces to reset. The code below
            // may reset the same surface up to 4 times :(
            for(auto ae = context->leftEdge.next; ae != &context->rightEdge; ae = ae->next)
            {
                if(ae->surfaces[0])
                {
                    ae->surfaces[0]->crossCount = 0;
                }

                if(ae->surfaces[1])
                {
                    ae->surfaces[1]->crossCount = 0;
                }
            }
        }
    }

    StopWatch::stop("scan-active-edge");

    if((context->renderContext->renderer->renderMode & 1) == 0)
        return;

    StopWatch::start("render-spans");

    for(X_AE_Surface* surface = context->surfaces.begin(); surface != context->surfaces.end(); ++surface)
    {
        if(surface->last == &surface->spanHead)
            continue;

        X_AE_SurfaceRenderContext surfaceRenderContext;
        x_ae_surfacerendercontext_init(&surfaceRenderContext, surface, context->renderContext, context->renderContext->renderer->mipLevel);
        x_ae_surfacerendercontext_render_spans(&surfaceRenderContext);
    }

    StopWatch::stop("render-spans");
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
