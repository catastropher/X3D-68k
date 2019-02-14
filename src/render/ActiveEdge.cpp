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

#include "error/Error.hpp"
#include "memory/Alloc.h"
#include "util/Util.hpp"
#include "ActiveEdge.hpp"
#include "level/BspLevel.hpp"
#include "geo/Polygon3.hpp"
#include "geo/Polygon2.hpp"
#include "Span.hpp"
#include "render/OldRenderer.hpp"
#include "engine/EngineContext.hpp"
#include "Camera.hpp"
#include "util/StopWatch.hpp"
#include "geo/Ray3.hpp"

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

X_AE_Edge* X_AE_Context::getCachedEdge(BspEdge* edge, int currentFrame) const
{
    X_AE_Edge* aeEdge = (X_AE_Edge*)((unsigned char*)edges.begin() + edge->cachedEdgeOffset);

    if(aeEdge->frameCreated != currentFrame || aeEdge->bspEdge != edge)
    {
        return NULL;
    }

    return aeEdge;
}

// TODO: no need to project vertices if can just reuse the cached edge
static bool project_polygon3(Polygon3* poly, Mat4x4* viewMatrix, Viewport* viewport, Polygon2* dest, fp& closestZ)
{
    const fp minZ = fp::fromFloat(0.5);

    closestZ = maxValue<fp>();

    for(int i = 0; i < poly->totalVertices; ++i)
    {
        Vec3fp transformed = viewMatrix->transform(poly->vertices[i]);

        if(transformed.z < minZ)
        {
            transformed.z = minZ;
        }
        
        poly->vertices[i] = transformed;
        
        closestZ = std::min(closestZ, transformed.z);

        viewport->project(transformed, dest->vertices[i]);
        viewport->clampfp(dest->vertices[i]);
    }
    
    dest->totalVertices = poly->totalVertices;

    return 1;
}

X_AE_Surface* X_AE_Context::createSurface(BspSurface* bspSurface, int bspKey)
{
    X_AE_Surface* surface = surfaces.alloc();

    surface->last = &surface->spanHead;
    surface->bspKey = bspKey;
    surface->bspSurface = bspSurface;
    surface->crossCount = 0;
    surface->closestZ = 0x7FFFFFFF;
    surface->modelOrigin = &currentModel->center;
    surface->flags = EnumBitSet<SurfaceFlags>(currentModel->flags);
    
    if(currentParent == NULL)
    {
        currentParent = surface;
    }
    
    surface->parent = currentParent;

    return surface;
}

void X_AE_Context::emitEdges(X_AE_Surface* surface, Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds)
{
    for(int i = 0; i < totalVertices; ++i)
    {
        int edgeId = abs(clippedEdgeIds[i]);
        BspEdge* bspEdge = currentModel->edges + edgeId;

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

bool projectAndClipBspPolygon(LevelPolygon3* poly, X_RenderContext* renderContext, BoundBoxFrustumFlags clipFlags, LevelPolygon2* dest, fp& closestZ)
{
    Vec3fp clippedV[X_POLYGON3_MAX_VERTS];
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

struct ClipContext
{
    ClipContext(X_Frustum* frustum_, int geoFlags_)
        : leftClipped(false),
        rightClipped(false),
        frustum(frustum_),
        geoFlags(geoFlags_),
        clipFlags(0),
        currentGroup(0)
    {
    }

    int clipRayToPlane(FrustumPlane* plane)
    {
        const int LEFT_PLANE = 0;
        const int RIGHT_PLANE = 1;

        int planeId = plane->id;

        fp v0DistToPlane;
        
        if(!wasClipped)
        {
            v0DistToPlane = dist[currentGroup ^ 1][planeId];
        }
        else
        {
            v0DistToPlane = plane->distanceTo(ray.v[0]);
        }

        int v0In = v0DistToPlane >= 0;
        
        fp v1DistToPlane = plane->distanceTo(ray.v[1]);
        int v1In = v1DistToPlane >= 0;

        dist[currentGroup][planeId] = v1DistToPlane;

        int flags = v0In | (v1In << 1);
        fp t;

        clipFlags &= flags;

        switch(flags)
        {
            case 0:
                break;

            case 1:
                t = v0DistToPlane / (v0DistToPlane - v1DistToPlane);
        
                ray.v[1] = ray.lerp(t);
                ray.v[0] = ray.v[0];

                if(planeId == LEFT_PLANE)
                {
                    leftClipped = true;
                    leftEnter = ray.v[1];
                }
                else if(planeId == RIGHT_PLANE)
                {
                    rightClipped = true;
                    rightEnter = ray.v[1];
                }

                break;

            case 2:
                t = v1DistToPlane / (v1DistToPlane - v0DistToPlane);
        
                ray.v[0] = ray.lerp(fp(X_FP16x16_ONE) - t);
                ray.v[1] = ray.v[1];

                if(planeId == LEFT_PLANE)
                {
                    leftClipped = true;
                    leftExit = ray.v[0];
                }
                else
                {
                    rightClipped = true;
                    rightExit = ray.v[0];
                }

                break;

            case 3:
                break;
        }

        return flags;
    }

    bool clip()
    {
        wasClipped = lastVertexWasClipped();
        clipFlags = 3;

        auto plane = frustum->head;

        while(plane)
        {
            if(clipRayToPlane(plane) == 0)
            {
                currentGroup ^= 1;

                return false;
            }

            plane = plane->next;
        };

        currentGroup ^= 1;

        return true;
    }

    bool clipToTopAndBottom()
    {
        if(clipRayToPlane(&frustum->planes[2]) == 0) return false;
        if(clipRayToPlane(&frustum->planes[3]) == 0) return false;

        return true;
    }

    bool lastVertexWasClipped()
    {
        return (clipFlags & 2) == 0;
    }
    
    Ray3 ray;

    bool leftClipped;
    bool rightClipped;

    Vec3fp leftEnter;
    Vec3fp leftExit;

    Vec3fp rightEnter;
    Vec3fp rightExit;

    X_Frustum* frustum;

    int geoFlags;

    Vec2 lastProjected;
    int clipFlags;

    fp dist[2][4];
    int currentGroup;

    bool wasClipped;
};

Vec3fp transform(Mat4x4& mat, Vec3fp v)
{
    return Vec3fp(
        mat.elem[0][0] * v.x + mat.elem[0][1] * v.y + mat.elem[0][2] * v.z + mat.elem[0][3],
        mat.elem[1][0] * v.x + mat.elem[1][1] * v.y + mat.elem[1][2] * v.z + mat.elem[1][3],
        mat.elem[2][0] * v.x + mat.elem[2][1] * v.y + mat.elem[2][2] * v.z + mat.elem[2][3]);
}

X_AE_Edge* X_AE_Context::addEdgeFromClippedRay(Ray3& clipped, X_AE_Surface* aeSurface, BspEdge* bspEdge, bool lastWasClipped, Vec2& lastProjected)
{
    Vec2_fp16x16 projected[2];

    if(!lastWasClipped)
    {
        projected[0] = lastProjected;
    }
    else
    {
        renderContext->cam->viewport.project(clipped.v[0], projected[0]);
        renderContext->cam->viewport.clampfp(projected[0]);
    }

    renderContext->cam->viewport.project(clipped.v[1], projected[1]);
    renderContext->cam->viewport.clampfp(projected[1]);

    lastProjected = projected[1];

    return addEdge(projected + 0, projected + 1, aeSurface, bspEdge);
}

void X_AE_Context::processPolygon(BspSurface* bspSurface,
                                  BoundBoxFrustumFlags geoFlags,
                                  int* edgeIds,
                                  int bspKey,
                                  bool inSubmodel)
{
    auto aeSurface = createSurface(bspSurface, bspKey); 

    int vertexIds[32];

    for(int i = 0; i < bspSurface->totalEdges; ++i)
    {
        if(!edge_is_flipped(edgeIds[i]))    vertexIds[i] = currentModel->edges[edgeIds[i]].v[0];
        else                                vertexIds[i] = currentModel->edges[-edgeIds[i]].v[1];
    }

    fp closestZ = maxValue<fp>();

    Vec3fp firstVertex = currentModel->vertices[vertexIds[0]].v;

    ClipContext context(renderContext->viewFrustum, (int)geoFlags);

    Vec3fp lastTransformed;
    Vec2 lastProjected;

    const fp minZ = fp::fromFloat(0.5);

    for(int i = 0; i < bspSurface->totalEdges; ++i)
    {
        auto bspEdge = currentModel->edges + abs(edgeIds[i]);

        int next = (i + 1 < bspSurface->totalEdges ? i + 1 : 0);

        context.ray.v[0] = currentModel->vertices[vertexIds[i]].v;
        context.ray.v[1] = currentModel->vertices[vertexIds[next]].v;

        bool lastWasClipped = context.lastVertexWasClipped();

        if(!context.clip())
        {
            continue;
        }

        if(lastWasClipped)
        {
            context.ray.v[0] = transform(*renderContext->viewMatrix, context.ray.v[0]);
        }
        else
        {
            context.ray.v[0] = lastTransformed;
        }

        context.ray.v[1] = transform(*renderContext->viewMatrix, context.ray.v[1]);

        lastTransformed = context.ray.v[1];

        for(int i = 0; i < 2; ++i)
        {
            closestZ = std::min(closestZ, context.ray.v[i].z);

            if(context.ray.v[i].z < minZ)
            {
                context.ray.v[i].z = minZ;
            }
        }

        // TODO: is there a reason we can't share clipped edges?
        if(context.clipFlags == 3)
        {
            auto cachedEdge = getCachedEdge(bspEdge, renderContext->currentFrame);

            if(cachedEdge != nullptr)
            {
                cachedEdge->emitCachedEdge(aeSurface);
                context.clipFlags = 0;
                continue;
            }
        }

        X_AE_Edge* edge = addEdgeFromClippedRay(context.ray, aeSurface, bspEdge, lastWasClipped, lastProjected);

        if(edge->isLeadingEdge)
        {
            // Vertices are swapped if leading edge
        }
    }

    if(context.leftClipped)
    {
        Ray3 ray(context.leftEnter, context.leftExit);

        context.ray = ray;
        context.wasClipped = true;

        if(context.clipToTopAndBottom())
        {
            ray = context.ray;

            for(int i = 0; i < 2; ++i)
            {
                ray.v[i] = transform(*renderContext->viewMatrix, ray.v[i]);

                if(ray.v[i].z < minZ)
                {
                    ray.v[i].z = minZ;
                }

                closestZ = std::min(closestZ, ray.v[i].z);
            }

            addEdgeFromClippedRay(ray, aeSurface, currentModel->edges + 0, true, lastProjected);
        }
    }

    if(context.rightClipped)
    {

        Ray3 ray(context.rightEnter, context.rightExit);

        context.ray = ray;
        context.wasClipped = true;

        if(context.clipToTopAndBottom())
        {
            for(int i = 0; i < 2; ++i)
            {
                // TODO: can just transform the Z because that's all we need
                ray.v[i] = transform(*renderContext->viewMatrix, ray.v[i]);
                closestZ = std::min(closestZ, ray.v[i].z);
            }
        }
    }

    aeSurface->inSubmodel = false;
    aeSurface->closestZ = closestZ.toFp16x16();
    
    aeSurface->calculateInverseZGradient(renderContext->camPos, &renderContext->cam->viewport, renderContext->viewMatrix, firstVertex);
}

void X_AE_Context::addPolygon(Polygon3* polygon, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int* edgeIds, int bspKey, bool inSubmodel)
{
    
    Vec3fp firstVertex = polygon->vertices[0];
    
    ++renderContext->renderer->totalSurfacesRendered;

    int clippedEdgeIds[X_POLYGON3_MAX_VERTS] = { 0 };
    
    LevelPolygon3 levelPoly(polygon->vertices, polygon->totalVertices, edgeIds);

    Vec2_fp16x16 v2d[X_POLYGON3_MAX_VERTS];
    LevelPolygon2 poly2d(v2d, X_POLYGON3_MAX_VERTS, clippedEdgeIds);
    
    fp closestZ;
    if(!projectAndClipBspPolygon(&levelPoly, renderContext, geoFlags, &poly2d, closestZ))
        return;
    
    X_AE_Surface* surface = createSurface(bspSurface, bspKey);

    surface->inSubmodel = inSubmodel;
    surface->closestZ = closestZ.toFp16x16();

    // FIXME: shouldn't be done this way
    
    surface->calculateInverseZGradient(renderContext->camPos, &renderContext->cam->viewport, renderContext->viewMatrix, firstVertex);
    emitEdges(surface, v2d, poly2d.totalVertices, poly2d.edgeIds);
}

static void get_model_polygon_from_edges(BspModel* model, int* edgeIds, int totalEdges, Polygon3* dest, Vec3fp* origin)
{
    dest->totalVertices = 0;
    
    for(int i = 0; i < totalEdges; ++i)
    {
        Vec3fp v;
        
        if(!edge_is_flipped(edgeIds[i]))
            v = model->vertices[model->edges[edgeIds[i]].v[0]].v;
        else
            v = model->vertices[model->edges[-edgeIds[i]].v[1]].v;
                
        dest->vertices[dest->totalVertices++] = v + *origin;
    }
}

void X_AE_Context::addLevelPolygon(BspLevel* level, int* edgeIds, int totalEdges, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(this);

    processPolygon(bspSurface, geoFlags, edgeIds, bspKey, false);

    return;


    
    InternalPolygon3 polygon;

    // FIXME: why is this here?
    polygon.totalVertices = 0;

    get_model_polygon_from_edges(currentModel, edgeIds, totalEdges, &polygon, &currentModel->center);

    addPolygon(&polygon, bspSurface, geoFlags, edgeIds, bspKey, 0);
}

void X_AE_Context::addSubmodelRecursive(Polygon3* poly, X_BspNode* node, int* edgeIds, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    if(!node->isVisibleThisFrame(renderContext->currentFrame))
        return;
    
    if(poly->totalVertices < 3)
        return;
    
    if(node->isLeaf())
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

void X_AE_Context::addSubmodelPolygon(BspLevel* level, int* edgeIds, int totalEdges, BspSurface* bspSurface, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(this);
    
    InternalPolygon3 poly;
    get_model_polygon_from_edges(currentModel, edgeIds, totalEdges, &poly, &currentModel->center);
    
    addSubmodelRecursive(&poly, x_bsplevel_get_root_node(level), edgeIds, bspSurface, geoFlags, bspKey);
}

X_AE_Surface* X_AE_Context::addBrushPolygon(Polygon3& polygon, Plane& polygonPlane, BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(this);

    int edgeIds[32] = { 0 };

    BspPlane bspPlane;
    bspPlane.plane = polygonPlane;

    BspSurface bspSurface;
    bspSurface.plane = &bspPlane;

    addSubmodelRecursive(&polygon, x_bsplevel_get_root_node(renderContext->level), edgeIds, &bspSurface, geoFlags, bspKey);

    return currentParent;
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
            int x = (edge->x).toInt();

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
        {
            return;
        }
        
        X_AE_Surface* search;
        if(surfaceToDisable != NULL)
            search = surfaceToDisable->prev;
        else
            search = topSurface;
        
        if(surfaceToEnable->isCloserThan(search, edge->x.toFp16x16(), y))
        {
            do
            {
                search = search->prev;
            } while(surfaceToEnable->isCloserThan(search, edge->x.toFp16x16(), y));
            
            
            search = search->next;
        }
        else
        {
            do
            {
                search = search->next;
            } while(!surfaceToEnable->isCloserThan(search, edge->x.toFp16x16(), y));
        }
        
        surfaceToEnable->next = search;
        surfaceToEnable->prev = search->prev;
        search->prev->next = surfaceToEnable;
        search->prev = surfaceToEnable;

        // Are we the top surface now?
        if(foreground.next == surfaceToEnable)
        {
            // Yes, emit span for the old top
            int x = edge->x.toInt();

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

    // If there is still a surface on top, we should emit a span for it

    if(foreground.next != &background)
    {
        if(screen->canvas.getW() - 1 > foreground.next->xStart)
        {
            emitSpan(foreground.next->xStart, screen->canvas.getW() - 1, y, foreground.next);
        }
    }

    // FIXME: temporary fix for disappearing polygons due to crossCount not getting reset
    // on each scanline. Should maybe keep a list of active surfaces to reset. The code below
    // may reset the same surface up to 4 times :(
    for(auto ae = leftEdge.next; ae != &rightEdge; ae = ae->next)
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
    //                             Vec2(edge->x >> 16, edge->startY),
    //                             Vec2((edge->x + edge->xSlope * (edge->endY - edge->startY)) >> 16, edge->endY),
    //                             context->screen->palette->brightRed);
    //     }
    //     else
    //     {
    //         x_texture_draw_line(&context->screen->canvas,
    //                             Vec2(edge->x >> 16, edge->startY),
    //                             Vec2((edge->x + edge->xSlope * (edge->endY - edge->startY)) >> 16, edge->endY),
    //                             context->screen->palette->darkGreen);
    //     }
    // }
    
    // return;
    
    static bool initialized = 0;

    if(!initialized)
    {
        x_console_register_var(context->renderContext->engineContext->console, &g_sortCount, "sortCount", X_CONSOLEVAR_INT, "0", 0);
        x_console_register_var(context->renderContext->engineContext->console, &g_stackCount, "stackCount", X_CONSOLEVAR_INT, "0", 0);
        initialized = 1;
    }

    context->resetBackgroundSurface();

    // Don't bother removing the edges from the last scanline
    context->newEdges[context->screen->getH() - 1].deleteHead = NULL;

    StopWatch::start("scan-active-edge");
    
    if((context->renderContext->renderer->renderMode & 2) != 0)
    {
        for(int i = 0; i < context->renderContext->cam->viewport.h; ++i)
        {
            context->processEdges(i);
        }
    }

    StopWatch::stop("scan-active-edge");

    if((context->renderContext->renderer->renderMode & 1) == 0)
        return;

    StopWatch::start("render-spans");

    int count = 0;

    for(X_AE_Surface* surface = context->surfaces.begin(); surface != context->surfaces.end(); ++surface)
    {
        if(surface->flags.hasFlag(SURFACE_NO_DRAW_SPANS))
        {
            ++count;
            continue;
        }

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

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, BspLevel* level)
{
    for(X_AE_Surface* s = context->surfaces.begin(); s != context->surfaces.end(); ++s)
    {
        if(x_ae_surface_point_is_in_surface_spans(s, x, y))
            return s->bspSurface - level->surfaces;
    }

    return -1;
}
