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

#include "level/X_BspLevel.h"
#include "X_Renderer.h"
#include "X_Surface.h"

#define GET_LUMEL(_x, _y) ((int)lumels[(_y) * lightW + (_x)] << 16)

static X_Color colorMap[256][256];
_Bool g_colorMapBuilt = 0;

static void rebuild_surface(X_BspSurface* surface, int mipLevel, X_Renderer* renderer)
{
    if(!g_colorMapBuilt)
    {
        const X_Palette* palette = x_palette_get_quake_palette();
        unsigned char r, g, b;
        
        for(int i = 0; i < 256; ++i)
        {
            x_palette_get_rgb(palette, i, &r, &g, &b);
            
            for(int j = 0; j < 256; ++j)
            {
                int rr = X_MIN(255, (r * j) >> 7);
                int gg = X_MIN(255, (g * j) >> 7);
                int bb = X_MIN(255, (b * j) >> 7);
                
                colorMap[i][j] = x_palette_get_closest_color_from_rgb(palette, rr, gg, bb);
            }
        }
        
        g_colorMapBuilt = 1;
    }
    
    int w = surface->textureExtent.x >> (mipLevel + 16);
    int h = surface->textureExtent.y >> (mipLevel + 16);
    int totalTexels = w * h;
    
    x_cache_alloc(&renderer->surfaceCache, totalTexels,  surface->cachedSurfaces + mipLevel); //x_malloc(totalTexels);
    
    int lightW = w / 16 + 1;
    int lightH = h / 16 + 1;
    
    unsigned char* lumels = surface->lightmapData;
    X_Color* texels = x_cache_get_cached_data(&renderer->surfaceCache, surface->cachedSurfaces + mipLevel);
    
    X_BspTexture* faceTex = surface->faceTexture->texture;
    
    X_Texture tex;
    tex.texels = faceTex->mipTexels[mipLevel];
    tex.w = faceTex->w >> mipLevel;
    tex.h = faceTex->h >> mipLevel;
    
    int minX = surface->textureMinCoord.x >> 16;
    int minY = surface->textureMinCoord.y >> 16;
    
    while(minX < 0) minX += tex.w;
    while(minY < 0) minY += tex.h;
    
    if(!renderer->enableLighting)
    {
        for(int i = 0; i < h; ++i)
        {
            for(int j = 0; j < w; ++j)
            {
                texels[i * w + j] = x_texture_get_texel(&tex, (j + minX) % tex.w, (i + minY) % tex.h);
            }
        }
        
        return;
    }
    
    for(int i = 0; i < lightH - 1; ++i)
    {
        for(int j = 0; j < lightW - 1; ++j)
        {
            int topLeft =       GET_LUMEL(j, i);
            int topRight =      GET_LUMEL(j + 1, i);
            int bottomLeft =    GET_LUMEL(j, i + 1);
            int bottomRight =   GET_LUMEL(j + 1, i + 1);
            
            int dLeft = (bottomLeft - topLeft) >> 4;
            int dRight = (bottomRight - topRight) >> 4;
            
            for(int k = 0; k < 16; ++k)
            {
                for(int d = 0; d < 16; ++d)
                {
                    int x = j * 16 + d;
                    int y = i * 16 + k;
                    
                    int left = topLeft + dLeft * k;
                    int right = topRight + dRight * k;
                    int dRow = (right - left) >> 4;
                    
                    X_Color texel = x_texture_get_texel(&tex, (x + minX) % tex.w, (y + minY) % tex.h);
                    
                    int intensity = (left + dRow * d) >> 16;
                    
                    texels[y * w + x] = colorMap[texel][intensity];
                }
            }
        }
    }
    
    //x_cache_alloc(&renderer->surfaceCache, totalTexels * sizeof(X_Color), surface->cachedSurfaces + mipLevel);
}

void x_bspsurface_get_surface_texture_for_mip_level(X_BspSurface* surface, int mipLevel, X_Renderer* renderer, X_Texture* dest)
{
    if(!x_cachentry_is_in_cache(surface->cachedSurfaces + mipLevel))
        rebuild_surface(surface, mipLevel, renderer);
    
    dest->w = surface->textureExtent.x >> (mipLevel + 16);
    dest->h = surface->textureExtent.y >> (mipLevel + 16);
    dest->texels = x_cache_get_cached_data(&renderer->surfaceCache, surface->cachedSurfaces + mipLevel);
}


