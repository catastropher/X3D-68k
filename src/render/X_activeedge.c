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
#include "X_span.h"
#include "render/X_Renderer.h"
#include "engine/X_EngineContext.h"

float g_zbuf[480][640];

int g_sortCount;
int g_stackCount;

static x_fp16x16 calculate_inverse_z(X_AE_Surface* s, x_fp16x16 x, int y)
{
    return x_fp16x16_mul(x, s->zInverseXStep) + y * s->zInverseYStep + s->zInverseOrigin;
}

static _Bool x_ae_surface_closer(X_AE_Surface* a, X_AE_Surface* b, x_fp16x16 x, int y, x_fp16x16 slope)
{
    if(a->bspKey != b->bspKey)
        return a->bspKey < b->bspKey;
    
    if(a->inSubmodel ^ b->inSubmodel)
        return a->inSubmodel ? 1 : 0;
    
    return calculate_inverse_z(a, x + X_FP16x16_ONE * 7, y) >= calculate_inverse_z(b, x + X_FP16x16_ONE * 7 , y);
}

static void x_ae_context_init_sentinal_edges(X_AE_Context* context)
{
    context->leftEdge.x = -0x7FFFFFFF;
    context->leftEdge.xSlope = 0;
    context->leftEdge.next = &context->rightEdge;

    context->rightEdge.x = 0x7FFFFFFF;
    context->rightEdge.xSlope = 0;
    context->rightEdge.prev = &context->leftEdge;
    context->rightEdge.next = NULL;
}

static void x_ae_context_init_edges(X_AE_Context* context, int maxActiveEdges, int edgePoolSize)
{
    context->edgePool = x_malloc(edgePoolSize * sizeof(X_AE_Edge));
    context->edgePoolEnd = context->edgePool + edgePoolSize;

    for(int i = 0; i < edgePoolSize; ++i)
    {
        context->edgePool[i].bspEdge = NULL;
        context->edgePool[i].frameCreated = -1;
    }

    context->newEdges = x_malloc(x_screen_h(context->screen) * sizeof(X_AE_DummyEdge));
}

static void x_ae_context_init_surfaces(X_AE_Context* context, int surfacePoolSize)
{
    context->surfacePool = x_malloc(surfacePoolSize * sizeof(X_AE_Surface));
    context->surfacePoolEnd = context->surfacePool + surfacePoolSize;

    for(int i = 0; i < surfacePoolSize; ++i)
        context->surfacePool[i].id = i;
}

void x_ae_context_init(X_AE_Context* context, X_Screen* screen, int maxActiveEdges, int edgePoolSize, int surfacePoolSize)
{
    context->screen = screen;

    x_ae_context_init_sentinal_edges(context);
    x_ae_context_init_edges(context, maxActiveEdges, edgePoolSize);
    x_ae_context_init_surfaces(context, surfacePoolSize);
}

void x_ae_context_cleanup(X_AE_Context* context)
{
    x_free(context->edgePool);
    x_free(context->newEdges);
    x_free(context->surfacePool);
}

static void x_ae_context_reset_active_edges(X_AE_Context* context)
{
    context->leftEdge.next = &context->rightEdge;
    context->leftEdge.prev = NULL;

    context->rightEdge.prev = &context->leftEdge;
    context->rightEdge.next = &context->newRightEdge;
}

static void x_ae_context_reset_background_surface(X_AE_Context* context)
{
    context->leftEdge.surfaces[X_AE_EDGE_RIGHT_SURFACE] = &context->background;
    context->leftEdge.surfaces[X_AE_EDGE_LEFT_SURFACE] = NULL;

    context->rightEdge.surfaces[X_AE_EDGE_LEFT_SURFACE] = &context->background;
    context->rightEdge.surfaces[X_AE_EDGE_RIGHT_SURFACE] = NULL;

    context->background.next = &context->background;
    context->background.prev = &context->background;
    context->background.bspKey = 0x7FFFFFFF;
    context->background.bspSurface = &context->backgroundBspSurface;
    context->background.parent = &context->background;
}

static void x_ae_context_reset_pools(X_AE_Context* context)
{
    context->nextAvailableEdge = context->edgePool;
    context->nextAvailableSurface = context->surfacePool;
}

static void x_ae_context_reset_new_edges(X_AE_Context* context)
{
    context->newRightEdge.x = context->rightEdge.x;
    context->newRightEdge.next = &context->newRightEdge;    // Loop back around

    // TODO: we can probably reset these as we sort in the new edges for each scanline
    for(int i = 0; i < x_screen_h(context->screen); ++i)
    {
        context->newEdges[i].next = (X_AE_Edge*)&context->newRightEdge;
        context->newEdges[i].x = x_fp16x16_from_float(-1000);
        context->newEdges[i].deleteHead = NULL;
    }
}

void x_ae_context_begin_render(X_AE_Context* context, X_RenderContext* renderContext)
{
    g_sortCount = 0;
    g_stackCount = 0;

    context->renderContext = renderContext;

    x_ae_context_reset_active_edges(context);
    x_ae_context_reset_pools(context);
    x_ae_context_reset_new_edges(context);
}

static _Bool x_ae_context_add_edge_to_starting_scanline(X_AE_Context* context, X_AE_Edge* newEdge, int startY, int endY)
{
    X_AE_Edge* edge = (X_AE_Edge*)&context->newEdges[startY];

    // FIXME: why is this check here?
    if(!edge)
        return 0;

    if(endY < 0)
        return 0;
    
    newEdge->nextDelete = context->newEdges[endY].deleteHead;
    context->newEdges[endY].deleteHead = newEdge;

    x_fp16x16 edgeX = newEdge->x;

    if(newEdge->surfaces[X_AE_EDGE_RIGHT_SURFACE] != NULL)
        ++edgeX;

    // TODO: the edges should be moved into an array in sorted - doing it a linked list is O(n^2)
    while(edge->next->x < edgeX)
    {
        edge = edge->next;
        ++g_sortCount;
    }

    newEdge->next = edge->next;
    edge->next = newEdge;
    
    return 1;
}

static _Bool is_horizontal_edge(int height)
{
    return height == 0;
}

static _Bool is_leading_edge(int height)
{
    return height < 0;
}

static void x_ae_edge_init_position_variables(X_AE_Edge* edge, X_Vec2_fp16x16* top, X_Vec2_fp16x16* bottom)
{
    edge->xSlope = x_fp16x16_div(bottom->x - top->x, bottom->y - top->y);

    x_fp16x16 topY = x_fp16x16_ceil(top->y);
    x_fp16x16 errorCorrection = x_fp16x16_mul(top->y - topY, edge->xSlope);

    edge->x = top->x - errorCorrection;
}

static X_AE_Edge* x_ae_context_allocate_edge(X_AE_Context* context)
{
    x_assert(context->nextAvailableEdge < context->edgePoolEnd, "AE out of edges");
    return context->nextAvailableEdge++;
}

X_AE_Edge* x_ae_context_add_edge(X_AE_Context* context, X_Vec2_fp16x16* a, X_Vec2_fp16x16* b, X_AE_Surface* surface, X_BspEdge* bspEdge)
{
    int aY = x_fp16x16_ceil(a->y) >> 16;
    int bY = x_fp16x16_ceil(b->y) >> 16;
    
    int height = bY - aY;

    if(is_horizontal_edge(height))
        return NULL;

    X_AE_Edge* edge = x_ae_context_allocate_edge(context);
    edge->isLeadingEdge = is_leading_edge(height);

    if(edge->isLeadingEdge)
        X_SWAP(a, b);

    edge->surfaces[X_AE_EDGE_RIGHT_SURFACE] = NULL;
    edge->surfaces[X_AE_EDGE_LEFT_SURFACE] = NULL;
    
    if(edge->isLeadingEdge)
        edge->surfaces[X_AE_EDGE_RIGHT_SURFACE] = surface;
    else
        edge->surfaces[X_AE_EDGE_LEFT_SURFACE] = surface;

    edge->frameCreated = context->renderContext->currentFrame;
    edge->bspEdge = bspEdge;

    bspEdge->cachedEdgeOffset = (unsigned char*)edge - (unsigned char*)context->edgePool;

    x_ae_edge_init_position_variables(edge, a, b);
    _Bool success = x_ae_context_add_edge_to_starting_scanline
    (
        context,
        edge,
        x_fp16x16_to_int(x_fp16x16_ceil(a->y)),
        x_fp16x16_to_int(x_fp16x16_ceil(b->y)) - 1
    );

    return success ? edge : NULL;
}

void x_ae_context_emit_cached_edge(X_AE_Edge* cachedEdge, X_AE_Surface* surface)
{
    if(cachedEdge->surfaces[X_AE_EDGE_LEFT_SURFACE] == NULL)
        cachedEdge->surfaces[X_AE_EDGE_LEFT_SURFACE] = surface;
    else if(cachedEdge->surfaces[X_AE_EDGE_RIGHT_SURFACE] == NULL)
        cachedEdge->surfaces[X_AE_EDGE_RIGHT_SURFACE] = surface;
    else
    {
        printf("Trying to emit full edge\n");
    }
}

static _Bool edge_is_flipped(int edgeId)
{
    return edgeId < 0;
}

static void x_bspsurface_calculate_plane_equation_in_view_space(X_BspSurface* surface, X_Vec3* camPos, X_Mat4x4* viewMatrix, X_Plane* dest, X_Vec3* pointOnSurface)
{
    X_Vec3 planeNormal = surface->plane->plane.normal;

    x_mat4x4_rotate_normal(viewMatrix, &planeNormal, &dest->normal);
    x_fp16x16 d = -x_vec3_dot(&planeNormal, pointOnSurface);
    
    dest->d = d + x_vec3_dot(&planeNormal, camPos);
}

static void x_ae_surface_calculate_inverse_z_gradient(X_AE_Surface* surface, X_Vec3* camPos, X_Viewport* viewport, X_Mat4x4* viewMatrix, X_Vec3* pointOnSurface)
{
    X_Plane planeInViewSpace;
    x_bspsurface_calculate_plane_equation_in_view_space(surface->bspSurface, camPos, viewMatrix, &planeInViewSpace, pointOnSurface);

    int dist = -planeInViewSpace.d;
    int scale = viewport->distToNearPlane;

    if(dist == 0 || scale == 0) return;
    
    //x_fp16x16 invDistTimesScale = //x_fp16x16_div(X_FP16x16_ONE << 10, distTimesScale) >> 6;
    
    x_fp16x16 invDist = x_fp16x16_div(X_FP16x16_ONE << 10, dist);
    x_fp16x16 invScale = (1 << 26) / scale;
    
    x_fp16x16 invDistTimesScale = x_fp16x16_mul(invDist, invScale) >> 10;
    
    surface->zInverseXStep = x_fp16x16_mul(invDistTimesScale, planeInViewSpace.normal.x);
    surface->zInverseYStep = x_fp16x16_mul(invDistTimesScale, planeInViewSpace.normal.y);

    int centerX = viewport->screenPos.x + viewport->w / 2;
    int centerY = viewport->screenPos.y + viewport->h / 2;

    surface->zInverseOrigin = x_fp16x16_mul(planeInViewSpace.normal.z, invDist) -
        centerX * surface->zInverseXStep -
        centerY * surface->zInverseYStep;
}

static X_AE_Edge* get_cached_edge(X_AE_Context* context, X_BspEdge* edge, int currentFrame)
{
    X_AE_Edge* aeEdge = (X_AE_Edge*)((unsigned char*)context->edgePool + edge->cachedEdgeOffset);

    if(aeEdge->frameCreated != currentFrame || aeEdge->bspEdge != edge)
        return NULL;

    return aeEdge;
}

static _Bool project_polygon3(X_Polygon3* poly, X_Mat4x4* viewMatrix, X_Viewport* viewport, X_AE_Surface* surface, X_Vec2* dest)
{
    for(int i = 0; i < poly->totalVertices; ++i)
    {
        X_Vec3 transformed;
        x_mat4x4_transform_vec3(viewMatrix, poly->vertices + i, &transformed);
        
        if(transformed.z < x_fp16x16_from_float(16.0))
            transformed.z = x_fp16x16_from_float(16.0);
        
        poly->vertices[i] = transformed;
        
        surface->closestZ = X_MIN(surface->closestZ, transformed.z);
        
        x_viewport_project_vec3(viewport, &transformed, dest + i);
        x_viewport_clamp_vec2_fp16x16(viewport, dest + i);
    }

    return 1;
}

static X_AE_Surface* create_ae_surface(X_AE_Context* context, X_BspSurface* bspSurface, int bspKey)
{
    X_AE_Surface* surface = context->nextAvailableSurface;
    
    if(context->nextAvailableSurface + 1 < context->surfacePoolEnd)
        ++context->nextAvailableSurface;

    surface->totalSpans = 0;
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

static void emit_edges(X_AE_Context* context, X_AE_Surface* surface, X_Vec2_fp16x16* v2d, int totalVertices, int* clippedEdgeIds)
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
                x_ae_context_emit_cached_edge(cachedEdge, surface);
                continue;
            }
        }

        int next = (i + 1 < totalVertices ? i + 1 : 0);
        x_ae_context_add_edge(context, v2d + i, v2d + next, surface, bspEdge);
    }
}

// TODO: check whether edgeIds is NULL
void x_ae_context_add_polygon(X_AE_Context* context, X_Polygon3* polygon, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int* edgeIds, int bspKey, _Bool inSubmodel)
{
    X_Vec3 clippedV[X_POLYGON3_MAX_VERTS];
    X_Polygon3 clipped = x_polygon3_make(clippedV, X_POLYGON3_MAX_VERTS);

    X_Vec3 firstVertex = polygon->vertices[0];
    
    ++context->renderContext->renderer->totalSurfacesRendered;

    int tempEdgeIds[X_POLYGON3_MAX_VERTS] = { 0 };
    int* clippedEdgeIds = tempEdgeIds;

    if(geoFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
    {
        // Don't bother clipping if fully inside the frustum
        clipped = *polygon;
        clippedEdgeIds = edgeIds;

    }
    else if(!x_polygon3_clip_to_frustum_edge_ids(polygon, context->renderContext->viewFrustum, &clipped, geoFlags, edgeIds, clippedEdgeIds))
    {
        return;
    }

    X_AE_Surface* surface = create_ae_surface(context, bspSurface, bspKey);

    surface->inSubmodel = inSubmodel;

    X_Vec2_fp16x16 v2d[X_POLYGON3_MAX_VERTS];
    if(!project_polygon3(&clipped, &context->renderContext->cam->viewMatrix, &context->renderContext->cam->viewport, surface, v2d))
        return;

    // FIXME: shouldn't be done this way
    X_Vec3 camPos = x_cameraobject_get_position(context->renderContext->cam);
    
    x_ae_surface_calculate_inverse_z_gradient(surface, &camPos, &context->renderContext->cam->viewport, context->renderContext->viewMatrix, &firstVertex);
    emit_edges(context, surface, v2d, clipped.totalVertices, clippedEdgeIds);
}

static void get_level_polygon_from_edges(X_BspLevel* level, int* edgeIds, int totalEdges, X_Polygon3* dest, X_Vec3* origin)
{
    dest->totalVertices = 0;
    
    for(int i = 0; i < totalEdges; ++i)
    {
        X_Vec3 v;
        
        if(!edge_is_flipped(edgeIds[i]))
            v = level->vertices[level->edges[edgeIds[i]].v[0]].v;
        else
            v = level->vertices[level->edges[-edgeIds[i]].v[1]].v;
                
        dest->vertices[dest->totalVertices++] = x_vec3_add(&v, origin);
    }
}

void x_ae_context_add_level_polygon(X_AE_Context* context, X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(context);
    
    if((context->renderContext->renderer->renderMode & 2) == 0)
        return;

    x_assert(context->nextAvailableSurface < context->surfacePoolEnd, "AE out of surfaces");

    X_Vec3 v3d[X_POLYGON3_MAX_VERTS];
    X_Polygon3 polygon = x_polygon3_make(v3d, 100);

    polygon.totalVertices = 0;

    if(!context->renderContext->renderer->frustumClip)
    {
        // Enable all clipping planes
        geoFlags = (1 << context->renderContext->viewFrustum->totalPlanes) - 1;
    }

    get_level_polygon_from_edges(level, edgeIds, totalEdges, &polygon, &context->currentModel->origin);

    x_ae_context_add_polygon(context, &polygon, bspSurface, geoFlags, edgeIds, bspKey, 0);
}

static void x_ae_context_add_submodel_polygon_recursive(X_AE_Context* context, X_Polygon3* poly, X_BspNode* node, int* edgeIds, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
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
    
    X_Vec3 frontVertices[32];
    X_Vec3 backVertices[32];
    int frontEdges[32] = { 0 };
    int backEdges[32] = { 0 };
    
    X_Polygon3 front = x_polygon3_make(frontVertices, 32);
    X_Polygon3 back = x_polygon3_make(backVertices, 32);
    
    x_polygon3_split_along_plane(poly, &node->plane->plane, edgeIds, &front, frontEdges, &back, backEdges);
    
    x_ae_context_add_submodel_polygon_recursive(context, &front, node->frontChild, frontEdges, bspSurface, geoFlags, bspKey);
    x_ae_context_add_submodel_polygon_recursive(context, &back, node->backChild, backEdges, bspSurface, geoFlags, bspKey);
}

void x_ae_context_add_submodel_polygon(X_AE_Context* context, X_BspLevel* level, int* edgeIds, int totalEdges, X_BspSurface* bspSurface, X_BoundBoxFrustumFlags geoFlags, int bspKey)
{
    x_ae_surface_reset_current_parent(context);
    
    X_Vec3 v3d[X_POLYGON3_MAX_VERTS];
    
    X_Polygon3 poly = x_polygon3_make(v3d, X_POLYGON3_MAX_VERTS);
    get_level_polygon_from_edges(level, edgeIds, totalEdges, &poly, &context->currentModel->origin);
    
    x_ae_context_add_submodel_polygon_recursive(context, &poly, x_bsplevel_get_root_node(level), edgeIds, bspSurface, geoFlags, bspKey);
}

static void x_ae_context_emit_span(int left, int right, int y, X_AE_Surface* surface)
{
    if(left == right)
        return;
    
    surface = surface->parent;

    X_AE_Span* span = surface->spans + surface->totalSpans;

    X_AE_Span* prev = surface->spans + surface->totalSpans - 1;
    
    // Faster way to check: (prev->y == y && prev->x2 == left)
    _Bool extendSpan = ((prev->y ^ y) | (prev->x2 ^ left)) == 0;
    
    if(extendSpan)
    {
        prev->x2 = right;
        return;
    }
    
    span->x1 = left;
    span->x2 = right;
    span->y = y;

    if(surface->totalSpans + 1 < X_AE_SURFACE_MAX_SPANS)
        ++surface->totalSpans;
}

static void remove_from_surface_stack(X_AE_Surface* surface)
{
    surface->prev->next = surface->next;
    surface->next->prev = surface->prev;
}

static inline void x_ae_context_process_edge(X_AE_Context* context, X_AE_Edge* edge, int y)
{
    X_AE_Surface* surfaceToEnable = edge->surfaces[X_AE_EDGE_RIGHT_SURFACE];
    X_AE_Surface* surfaceToDisable = edge->surfaces[X_AE_EDGE_LEFT_SURFACE];

    X_AE_Surface* topSurface = context->background.next;

    if(surfaceToDisable != NULL)
    {
        // Make sure the edges didn't cross
        if(--surfaceToDisable->crossCount != 0)
            goto enable;

        
        if(surfaceToDisable == topSurface)
        {
            // We were on top, so emit the span
            int x = (edge->x) >> 16;

            x_ae_context_emit_span(surfaceToDisable->xStart, x, y, surfaceToDisable);
            
            topSurface->next->xStart = x;
        }
        
        remove_from_surface_stack(surfaceToDisable);
    }

enable:
    if(surfaceToEnable != NULL)
    {
        topSurface = context->background.next;
        
        // Make sure the edges didn't cross
        if(++surfaceToEnable->crossCount != 1)
            return;

        // Are we the top surface now?
        if(x_ae_surface_closer(surfaceToEnable, topSurface, edge->x, y, edge->xSlope))
        {
            // Yes, emit span for the current top
            int x = edge->x >> 16;

            x_ae_context_emit_span(topSurface->xStart, x, y, topSurface);

            surfaceToEnable->xStart = x;
            
            surfaceToEnable->next = topSurface;
            surfaceToEnable->prev = &context->background;
            
            topSurface->prev = surfaceToEnable;
            context->background.next = surfaceToEnable;
        }
        else
        {
            do
            {
                topSurface = topSurface->next;
            } while(!x_ae_surface_closer(surfaceToEnable, topSurface, edge->x, y, edge->xSlope));
            
            surfaceToEnable->next = topSurface;
            surfaceToEnable->prev = topSurface->prev;
            topSurface->prev->next = surfaceToEnable;
            topSurface->prev = surfaceToEnable;
        }
    }
}

static inline void x_ae_context_add_active_edge(X_AE_Context* context, X_AE_Edge* edge, int y)
{
    x_ae_context_process_edge(context, edge, y);

    // Advance the edge
    edge->x += edge->xSlope;

    // Resort the edge, if it moved out of order
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
    context->background.next = &context->background;
    context->background.prev = &context->background;

    X_AE_Edge* activeEdge = context->leftEdge.next;

    X_AE_Edge* newEdge = context->newEdges[y].next;

    int total = 0;

    do
    {
        X_AE_Edge* e;

        if(activeEdge->x <= newEdge->x)
        {
            e = activeEdge;
            activeEdge = activeEdge->next;
        }
        else
        {
            X_AE_Edge* prev = activeEdge->prev;

            prev->next = newEdge;
            newEdge->prev = prev;

            X_AE_Edge* newEdgeNext = newEdge->next;
            newEdge->next = activeEdge;
            activeEdge->prev = newEdge;

            e = newEdge;
            newEdge = newEdgeNext;
        }

        ++total;

        if(e->next == &context->newRightEdge)
            break;

        x_ae_context_add_active_edge(context, e, y);
    } while(1);

    g_stackCount = X_MAX(g_stackCount, total);

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
    static _Bool initialized = 0;

    if(!initialized)
    {
        x_console_register_var(&context->renderContext->engineContext->console, &g_sortCount, "sortCount", X_CONSOLEVAR_INT, "0", 0);
        x_console_register_var(&context->renderContext->engineContext->console, &g_stackCount, "stackCount", X_CONSOLEVAR_INT, "0", 0);
        initialized = 1;
    }

    x_ae_context_reset_background_surface(context);

    if((context->renderContext->renderer->renderMode & 2) != 0)
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

    if((context->renderContext->renderer->renderMode & 1) == 0)
        return;

    for(int i = 0; i < context->nextAvailableSurface - context->surfacePool; ++i)
    {
        if(context->surfacePool[i].totalSpans == 0)
            continue;

        X_AE_SurfaceRenderContext surfaceRenderContext;
        x_ae_surfacerendercontext_init(&surfaceRenderContext, context->surfacePool + i, context->renderContext, context->renderContext->renderer->mipLevel);
        x_ae_surfacerendercontext_render_spans(&surfaceRenderContext);
    }
}

_Bool x_ae_surface_point_is_in_surface_spans(X_AE_Surface* surface, int x, int y)
{
    for(int i = 0; i < surface->totalSpans; ++i)
    {
        X_AE_Span* span = surface->spans + i;
        if(span->y == y && x >= span->x1 && x < span->x2)
            return 1;
    }

    return 0;
}

int x_ae_context_find_surface_point_is_in(X_AE_Context* context, int x, int y, X_BspLevel* level)
{
    for(int i = 0; i < context->nextAvailableSurface - context->surfacePool; ++i)
    {
        if(x_ae_surface_point_is_in_surface_spans(context->surfacePool + i, x, y))
            return context->surfacePool[i].bspSurface - level->surfaces;
    }

    return -1;
}
