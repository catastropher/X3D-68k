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

static void fill_solid_span(X_RenderContext* context, X_TriangleFillerEdge* left, X_TriangleFillerEdge* right, int y, X_Color color)
{
    int leftX = x_fp16x16_to_int(left->x);
    int rightX = x_fp16x16_to_int(right->x);
    int dx = rightX - leftX;
    
    if(dx <= 0)
        return;
    
    x_fp16x16 z = left->z;
    x_fp16x16 dZ = (right->z - left->z) / dx;
    
    int screenW = x_screen_w(context->screen);
    int screenH = x_screen_h(context->screen);
    
    x_fp0x16* zbuf = context->screen->canvas.zbuf + y * screenW;
    
    if(leftX < 0 || rightX >= screenW)
        return;
    
    if(y < 0 || y >= screenH)
        return;
    
    for(int i = leftX; i < rightX; ++i)
    {
        if(z >= zbuf[i] << X_TRIANGLEFILLER_EXTRA_PRECISION)
        {
            x_texture_set_texel(&context->screen->canvas.tex, i, y, color);
            zbuf[i] = z >> X_TRIANGLEFILLER_EXTRA_PRECISION;
        }
        
        z += dZ;
    }
        
}

static void fill_flat_shaded_draw_half(X_TriangleFiller* filler)
{
    X_TriangleFillerEdge* leftEdge = filler->leftEdge;
    X_TriangleFillerEdge* rightEdge = filler->rightEdge;
    
    while(filler->y < filler->endY)
    {
        fill_solid_span(filler->renderContext, leftEdge, rightEdge, filler->y, filler->fillColor);
        
        leftEdge->x += leftEdge->xSlope;
        leftEdge->z += leftEdge->zSlope;
        
        rightEdge->x += rightEdge->xSlope;
        rightEdge->z += rightEdge->zSlope;
        
        ++filler->y;
    }
}

static void switch_to_bottom_half_of_triangle(X_TriangleFiller* filler)
{
    *filler->firstEndingEdge = filler->middleStartingEdge;
    filler->endY = filler->middleStartingEdge->endY;
}

static void add_left_edge(X_TriangleFiller* filler, X_TriangleFillerEdge* newEdge)
{
    if(filler->leftEdge == NULL)
    {
        filler->leftEdge = newEdge;
        return;
    }
    
    filler->firstEndingEdge = &filler->leftEdge;
    
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
    
    filler->firstEndingEdge = &filler->rightEdge;
    
    if(filler->rightEdge->endY < newEdge->endY)
    {
        filler->middleStartingEdge = newEdge;
    }
    else
    {
        filler->middleStartingEdge = filler->rightEdge;
        filler->rightEdge = newEdge;
    }
}

static void init_edge_slopes(X_TriangleFillerEdge* edge, X_TriangleFillerVertex* a, X_TriangleFillerVertex* b)
{
    if(a->v.y > b->v.y)
        X_SWAP(a, b);
    
    int dy = b->v.y - a->v.y;
    edge->x = x_fp16x16_from_int(a->v.x) + X_FP16x16_HALF;
    edge->xSlope = x_int_div_as_fp16x16(b->v.x - a->v.x, dy);
    
    edge->z = a->z;
    edge->zSlope = (b->z - a->z) / dy;
    
    edge->endY = b->v.y;
}

static void add_edge(X_TriangleFiller* filler, X_TriangleFillerVertex* a, X_TriangleFillerVertex* b)
{
    filler->y = X_MIN(filler->y, a->v.y);
    filler->y = X_MIN(filler->y, b->v.y);
    
    X_TriangleFillerEdge* newEdge = filler->edges + filler->nextEdge++;
    
    int height = b->v.y - a->v.y;
    if(height == 0)
    {
        filler->type = X_TRIANGLE_FLAT;
        return;
    }
    
    _Bool isLeftEdge = height < 0;
    
    init_edge_slopes(newEdge, a, b);
    
    if(isLeftEdge)
        add_left_edge(filler, newEdge);
    else
        add_right_edge(filler, newEdge);
}

static void init_edges(X_TriangleFiller* filler)
{
    filler->type = X_TRIANGLE_GENERIC;
    
    for(int i = 0; i < 3; ++i)
    {
        int next = (i + 1 < 3 ? i + 1 : 0);
        add_edge(filler, filler->vertices + i, filler->vertices + next);
    }
}

void x_trianglefiller_fill_flat_shaded(X_TriangleFiller* filler, X_Color color)
{
    filler->fillColor = color;
    init_edges(filler);
    
    if(!filler->leftEdge || !filler->rightEdge)
        return;
    
    if(filler->leftEdge->x + filler->leftEdge->xSlope > filler->rightEdge->x + filler->rightEdge->xSlope)
        return;
    
    if(filler->type != X_TRIANGLE_GENERIC)
    {
        filler->endY = filler->leftEdge->endY;
        fill_flat_shaded_draw_half(filler);
        return;
    }
    
    filler->endY = (*filler->firstEndingEdge)->endY;
    
    fill_flat_shaded_draw_half(filler);
    switch_to_bottom_half_of_triangle(filler);
    fill_flat_shaded_draw_half(filler);
}

void x_trianglefiller_init(X_TriangleFiller* filler, X_RenderContext* renderContext)
{
    filler->renderContext = renderContext;
    filler->nextEdge = 0;
    filler->rightEdge = NULL;
    filler->leftEdge = NULL;
    filler->y = 0xFFFF;
}

