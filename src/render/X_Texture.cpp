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

#include <algorithm>

#include "X_Texture.h"
#include "system/X_File.h"
#include "error/X_log.h"
#include "util/X_util.h"
#include "X_Font.h"

bool X_Texture::saveToFile(const char* fileName)
{
    X_File file;
    if(!x_file_open_writing(&file, fileName))
        return 0;
    
    x_file_write_buf(&file, 4, (void*)"XTEX");
    x_file_write_le_int16(&file, w);
    x_file_write_le_int16(&file, h);
    x_file_write_buf(&file, totalTexels(), texels);
    
    x_file_close(&file);
    
    return 1;
}

bool X_Texture::loadFromFile(const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return 0;
    
    char signature[5];
    x_file_read_fixed_length_str(&file, 4, signature);
    
    if(strcmp(signature, "XTEX") != 0)
    {
        x_log_error("File %s has bad XTEX header", fileName);
        return 0;
    }
    
    int w = x_file_read_le_int16(&file);
    int h = x_file_read_le_int16(&file);
    
    resize(w, h);
    x_file_read_buf(&file, totalTexels(), texels);
    
    x_file_close(&file);
    
    return 1;
}

void X_Texture::clampVec2i(X_Vec2i& v)
{
    v.x = std::max(v.x, 0);
    v.x = std::min(v.x, w - 1);
    
    v.y = std::max(v.y, 0);
    v.y = std::min(v.y, h - 1);
}

void X_Texture::drawLine(X_Vec2i start, X_Vec2i end, X_Color color)
{
    clampVec2i(start);
    clampVec2i(end);
    
    int dx = abs(end.x - start.x);
    int sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y);
    int sy = start.y < end.y ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 2;
    X_Vec2i pos = start;
    
    while(1)
    {
        setTexel(pos, color);
        
        if(x_vec2_equal(&pos, &end))
            break;
        
        int old_err = err;
        if (old_err > -dx)
        {
            err -= dy;
            pos.x += sx;
        }
        
        if (old_err < dy)
        {
            err += dx;
            pos.y += sy;
        }
    }
}

void X_Texture::blit(const X_Texture& tex, X_Vec2i pos)
{
    int endX = std::min(pos.x + tex.w, w);
    int endY = std::min(pos.y + tex.h, h);
    
    for(int y = pos.y; y < endY; ++y)
    {
        for(int x = pos.x; x < endX; ++x)
        {
            setTexel({ x, y }, tex.getTexel({ x - pos.x, y - pos.y }));
        }
    }
}

void X_Texture::drawChar(int c, const X_Font& font, X_Vec2i pos)
{
    const X_Color* charPixels = x_font_get_character_pixels(&font, c);

    X_Vec2 clippedTopLeft
    {
        std::max(0, pos.x) - pos.x,
        std::max(0, pos.y) - pos.y
    };

    X_Vec2 clippedBottomRight
    {
        std::min(w, pos.x + font.charW) - pos.x,
        std::min(h, pos.y + font.charH) - pos.y
    };
    
    for(int i = clippedTopLeft.y; i < clippedBottomRight.y; ++i)
    {
        for(int j = clippedTopLeft.x; j < clippedBottomRight.x; ++j)
        {
            setTexel({ pos.x + j, pos.y + i }, *charPixels++);
        }
    }
}

void X_Texture::drawStr(const char* str, const X_Font& font, X_Vec2i pos)
{
    X_Vec2 currentPos = pos;
    
    while(*str)
    {
        if(*str == '\n')
        {
            currentPos.x = pos.x;
            currentPos.y += font.charH;
        }
        else
        {
            drawChar(*str, font, currentPos);
            currentPos.x += font.charW;
        }
        
        ++str;
    }
}

void X_Texture::fillRect(X_Vec2i topLeft, X_Vec2i bottomRight, X_Color color)
{
    clampVec2i(topLeft);
    clampVec2i(bottomRight);
    
    for(int y = topLeft.y; y <= bottomRight.y; ++y)
    {
        for(int x = topLeft.x; x <= bottomRight.x; ++x)
        {
            setTexel({ x, y }, color);
        }
    }
}

void X_Texture::fill(X_Color fillColor)
{
    memset(texels, fillColor, totalTexels());
}

// static void construct_vertices_of_decal_polygon(X_Vec2_fp16x16* dest, X_Texture* decal, X_Vec2 pos, X_Vec2_fp16x16* uOrientation, X_Vec2_fp16x16* vOrientation)
// {
//     int w = decal.w / 2;
//     int h = decal.h / 2;
    
//     dest[0] = x_vec2_make(-w / 2, -h / 2);
//     dest[1] = x_vec2_make(w / 2, -h / 2);
//     dest[2] = x_vec2_make(w / 2, h / 2);
//     dest[3] = x_vec2_make(-w / 2, h / 2);
    
//     for(int i = 0; i < 4; ++i)
//     {
//         X_Vec2 v = dest[i];
        
//         dest[i].x = v.x * uOrientation.x + v.y * uOrientation.y + x_fp16x16_from_int(pos.x);
//         dest[i].y = v.x * vOrientation.x + v.y * vOrientation.y + x_fp16x16_from_int(pos.y);
//     }
// }

// static void calculate_texture_coordinates(X_Vec2_fp16x16* dest, X_Texture* decal)
// {
//     x_fp16x16 w = x_fp16x16_from_int(decal.w - 1);
//     x_fp16x16 h = x_fp16x16_from_int(decal.h - 1);
    
//     dest[0] = x_vec2_make(0, 0);
//     dest[1] = x_vec2_make(w, 0);
//     dest[2] = x_vec2_make(w, h);
//     dest[3] = x_vec2_make(0, h);
// }

// typedef struct X_DecalEdge
// {;
//     x_fp16x16 x;
//     x_fp16x16 xSlope;
//     x_fp16x16 u;
//     x_fp16x16 uSlope;
//     x_fp16x16 v;
//     x_fp16x16 vSlope;
 
//     int startY;
//     int endY;
//     bool isLeadingEdge;
    
//     struct X_DecalEdge* next;
// } X_DecalEdge;

// static void init_slope_with_error_correction(x_fp16x16 x0, x_fp16x16 y0, x_fp16x16 x1, x_fp16x16 y1, x_fp16x16* x, x_fp16x16* slope)
// {
//     *slope = x_fp16x16_div(x1 - x0, y1 - y0);
    
//     x_fp16x16 topY = x_fp16x16_ceil(y0);
//     x_fp16x16 errorCorrection = x_fp16x16_mul(y0 - topY, *slope);
    
//     *x = x0 - errorCorrection;
// }

// static void clip_edge(X_DecalEdge* edge, x_fp16x16 canvasHeight)
// {
//     if(edge.endY >= x_fp16x16_to_int(canvasHeight))
//         edge.endY = x_fp16x16_to_int(canvasHeight - X_FP16x16_ONE);
    
//     if(edge.startY < 0)
//     {
//         int dY = -edge.startY;
//         edge.x += edge.xSlope * dY;
//         edge.u += edge.uSlope * dY;
//         edge.v += edge.vSlope * dY;
//         edge.startY = 0;
//     }
        
// }

// static bool x_decaledge_init(X_DecalEdge* edge, X_Vec2_fp16x16* v, X_Vec2_fp16x16* texCoords, int aIndex, int bIndex, x_fp16x16 canvasHeight)
// {
//     x_fp16x16 aY = x_fp16x16_ceil(v[aIndex].y);
//     x_fp16x16 bY = x_fp16x16_ceil(v[bIndex].y);
    
//     x_fp16x16 height = bY - aY;
//     if(height == 0)
//         return 0;
    
//     if(aY < 0 && bY < 0)
//         return 0;
    
//     if(aY >= canvasHeight && bY >= canvasHeight)
//         return 0;
    
//     edge.isLeadingEdge = (height < 0);
    
//     if(edge.isLeadingEdge)
//         X_SWAP(aIndex, bIndex);
    
//     init_slope_with_error_correction(v[aIndex].x, v[aIndex].y, v[bIndex].x, v[bIndex].y, &edge.x, &edge.xSlope);
//     init_slope_with_error_correction(texCoords[aIndex].x, v[aIndex].y, texCoords[bIndex].x, v[bIndex].y, &edge.u, &edge.uSlope);
//     init_slope_with_error_correction(texCoords[aIndex].y, v[aIndex].y, texCoords[bIndex].y, v[bIndex].y, &edge.v, &edge.vSlope);
    
//     edge.startY = x_fp16x16_to_int(x_fp16x16_ceil(v[aIndex].y));
//     edge.endY = x_fp16x16_to_int(x_fp16x16_ceil(v[bIndex].y)) - 1;
    
//     clip_edge(edge, canvasHeight);
    
//     return 1;
// }

// static void init_sentinel_edges(X_DecalEdge* head, X_DecalEdge* tail)
// {
//     head.endY = -0x7FFFFFFF;
//     tail.endY = 0x7FFFFFFF;
    
//     head.next = tail;
//     tail.next = NULL;
// }

// void insert_edge(X_DecalEdge* head, X_DecalEdge* edge)
// {
//     while(head.next.endY < edge.endY)
//         head = head.next;
    
//     edge.next = head.next;
//     head.next = edge;
// }

// static void add_edges(X_DecalEdge* edges, X_Vec2_fp16x16* v, X_Vec2_fp16x16* texCoords, X_DecalEdge* leftHead, X_DecalEdge* rightHead, x_fp16x16 canvasHeight)
// {    
//     for(int i = 0; i < 4; ++i)
//     {
//         int next = (i + 1 < 4 ? i + 1 : 0);
//         if(!x_decaledge_init(edges + i, v, texCoords, i, next, canvasHeight))
//             continue;
     
//         if(edges[i].isLeadingEdge)
//             insert_edge(leftHead, edges + i);
//         else
//             insert_edge(rightHead, edges + i);
//     }
// }

// static void draw_span(X_Texture* canvas, X_Texture* decal, X_DecalEdge* left, X_DecalEdge* right, int y, X_Color transparency)
// {
//     int xLeft = x_fp16x16_to_int(left.x);
//     int xRight = x_fp16x16_to_int(right.x);
    
//     int dX = X_MAX(xRight - xLeft, 1);
    
//     x_fp16x16 u = left.u;
//     x_fp16x16 dU = (right.u - left.u) / dX;
    
//     x_fp16x16 v = left.v;
//     x_fp16x16 dV = (right.v - left.v) / dX;
    
//     if(xLeft < 0)
//     {
//         u -= xLeft * dU;
//         v -= xLeft * dV;
//         xLeft = 0;
//     }
    
//     xRight = X_MIN(xRight, canvas.w - 1);
    
//     for(int i = xLeft; i <= xRight; ++i)
//     {        
//         X_Color texel = x_texture_get_texel(decal, x_fp16x16_to_int(u), x_fp16x16_to_int(v));
        
//         if(texel != transparency)
//             x_texture_set_texel(canvas, i, y, texel);
        
//         u += dU;
//         v += dV;
//     }
// }

// static void advance_edge(X_DecalEdge* edge)
// {
//     edge.x += edge.xSlope;
//     edge.u += edge.uSlope;
//     edge.v += edge.vSlope;
// }

// static void scan_edges(X_Texture* canvas, X_Texture* decal, X_DecalEdge* left, X_DecalEdge* right, X_Color transparency)
// {
//     int y = left.startY;
//     X_DecalEdge** nextAdvance;
    
//     do
//     {
//         nextAdvance = (left.endY < right.endY ? &left : &right);
//         int nextEndY = (*nextAdvance).endY;
        
//         do
//         {
//             draw_span(canvas, decal, left, right, y, transparency);
//             advance_edge(left);
//             advance_edge(right);
//             ++y;
//         } while(y <= nextEndY);
        
//         *nextAdvance = (*nextAdvance).next;
//     } while((*nextAdvance).next != NULL);   
// }

// void x_texture_draw_decal(X_Texture* canvas, X_Texture* decal, X_Vec2 pos, X_Vec2_fp16x16* uOrientation, X_Vec2_fp16x16* vOrientation, X_Color transparency)
// {
//     X_DecalEdge leftHead, leftTail;
//     init_sentinel_edges(&leftHead, &leftTail);
    
//     X_DecalEdge rightHead, rightTail;
//     init_sentinel_edges(&rightHead, &rightTail);
    
//     X_Vec2_fp16x16 v[4];
//     construct_vertices_of_decal_polygon(v, decal, pos, uOrientation, vOrientation);
    
//     X_Vec2_fp16x16 texCoords[4];
//     calculate_texture_coordinates(texCoords, decal);
    
//     X_DecalEdge edges[4];
//     add_edges(edges, v, texCoords, &leftHead, &rightHead, x_fp16x16_from_int(canvas.h));
    
//     if(leftHead.next != &leftTail)
//         scan_edges(canvas, decal,leftHead.next, rightHead.next, transparency);
// }

