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

#include "X_TriangleFiller.h"

static void fill_solid_span(X_RenderContext* context, x_fp16x16 x, x_fp16x16 right, X_Color color)
{
    
}

static void fill_flat_shaded_draw_half(X_TriangleFiller* filler)
{
    X_TriangleFillerEdge* leftEdge = filler->leftEdge;
    X_TriangleFillerEdge* rightEdge = filler->leftEdge;
    
    while(filler->y < filler->endY)
    {
        fill_solid_span(filler->renderContext, leftEdge->x, rightEdge->x, filler->fillColor);
        leftEdge->x += leftEdge->xSlope;
        rightEdge->x += rightEdge->xSlope;
        ++filler->y;
    }
}

static void switch_to_bottom_half_of_triangle(X_TriangleFiller* filler)
{
    *filler->firstEndingEdge = &filler->edges[2];
    filler->endY = filler->edges[2].endY;
}

static void add_left_edge(X_TriangleFiller* filler, X_TriangleFillerEdge* newEdge)
{
    if(filler->leftEdge == NULL)
    {
        filler->leftEdge = newEdge;
        return;
    }
    
    if(filler->leftEdge->endY < newEdge->endY)
    {
        filler->middleStartingEdge = newEdge;
    }
    else
    {
        filler->middleStartingEdge = filler->leftEdge;
        filler->leftEdge = newEdge;
    }
}

static void add_right_edge(X_TriangleFiller* filler, X_TriangleFillerEdge* newEdge)
{
    if(filler->rightEdge == NULL)
    {
        filler->rightEdge = newEdge;
        return;
    }
    
    if(filler->rightEdge->endY < newEdge->endY)
    {
        filler->middleStartingEdge = newEdge;
    }
    else
    {
        filler->middleStartingEdge = filler->leftEdge;
        filler->rightEdge = newEdge;
    }
}

static void init_edge_slopes(X_TriangleFillerEdge* edge, X_TriangleFillerVertex* a, X_TriangleFillerVertex* b)
{
    if(a->v.y > b->v.y)
        X_SWAP(a, b);
    
    edge->x = x_fp16x16_from_int(a->v.x);
    edge->xSlope = x_int_div_as_fp16x16(b->v.x - a->v.x, b->v.y - a->v.y);
    edge->endY = b->v.y;
}

static void add_edge(X_TriangleFiller* filler, X_TriangleFillerVertex* a, X_TriangleFillerVertex* b)
{
    X_TriangleFillerEdge* newEdge = filler->edges + filler->nextEdge++;
    init_edge_slopes(newEdge, a, b);
    
    int height = b->v.y - a->v.y;
    _Bool isLeftEdge = height < 0;
    
    if(isLeftEdge)
        add_left_edge(filler, newEdge);
    else
        add_right_edge(filler, newEdge);
}

static void init_edges(X_TriangleFiller* filler)
{
    for(int i = 0; i < 3; ++i)
    {
        int next = (i + 1 < 3 ? i + 1 : 0);
        add_edge(filler, filler->vertices + i, filler->vertices + next);
    }
}

void x_trianglefiller_fill_flat_shaded(X_TriangleFiller* filler, X_Color color)
{
    init_edges(filler);
    
    if(filler->type != X_TRIANGLE_GENERIC)
    {
        fill_flat_shaded_draw_half(filler);
        return;
    }
    
    filler->endY = (*filler->firstEndingEdge)->endY;
    
    fill_flat_shaded_draw_half(filler);
    switch_to_bottom_half_of_triangle(filler);
    fill_flat_shaded_draw_half(filler);
}

