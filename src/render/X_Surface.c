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

#define GET_LUMEL(_x, _y) ((int)combinedLightmap[(_y) * lightW + (_x)] << 16)

#define SHADES_PER_COLOR 64

static X_Color colorMap[256][SHADES_PER_COLOR];
_Bool g_colorMapBuilt = 0;

static int convert_shade(int intensity, int shade)
{
    return (intensity * (shade) + 16) / 32;
}

#define X_LIGHTMAP_MAX_SIZE 34

static _Bool is_power_of_2(int val)
{
    return val != 0 && (val & (val - 1)) == 0;
}

static void rebuild_surface(X_BspSurface* surface, int mipLevel, X_Renderer* renderer)
{
    int combinedLightmap[X_LIGHTMAP_MAX_SIZE * X_LIGHTMAP_MAX_SIZE];
    
    if(!g_colorMapBuilt)
    {
        const int TOTAL_FULLBRIGHTS = 32;
        
        const X_Palette* palette = x_palette_get_quake_palette();
        unsigned char r, g, b;
        
        for(int i = 0; i < 256 - TOTAL_FULLBRIGHTS; ++i)
        {
            x_palette_get_rgb(palette, i, &r, &g, &b);
            
            for(int j = 0; j < SHADES_PER_COLOR; ++j)
            {
                int rr = X_MIN(255, convert_shade(r, j));
                int gg = X_MIN(255, convert_shade(g, j));
                int bb = X_MIN(255, convert_shade(b, j));
                
                colorMap[i][j] = x_palette_get_closest_color_from_rgb(palette, rr, gg, bb);
            }
        }
        
        for(int i = 256 - TOTAL_FULLBRIGHTS; i < 256; ++i)
        {
            for(int j = 0; j < SHADES_PER_COLOR; ++j)
                colorMap[i][j] = i;
        }
        
        g_colorMapBuilt = 1;
    }
    
    int w = surface->textureExtent.x >> (mipLevel + 16);
    int h = surface->textureExtent.y >> (mipLevel + 16);
    int totalTexels = w * h;
    
    x_cache_alloc(&renderer->surfaceCache, totalTexels,  surface->cachedSurfaces + mipLevel); //x_malloc(totalTexels);
    
    int lightW = w / 16 + 1;
    int lightH = h / 16 + 1;
    
    int lightSize = lightW * lightH;
    
    // Clear to ambient
    for(int i = 0; i < lightSize; ++i)
        combinedLightmap[i] = 0;
    
    unsigned char* lumels = surface->lightmapData;
    
    for(int i = 0; i < X_BSPSURFACE_MAX_LIGHTMAPS; ++i)
    {
        if(surface->lightmapStyles[i] == 255)
            break;
        
        for(int j = 0; j < lightSize; ++j)
            combinedLightmap[j] += lumels[j];
        
        lumels += lightSize;
    }
    
    X_Color* texels = x_cache_get_cached_data(&renderer->surfaceCache, surface->cachedSurfaces + mipLevel);
    
    X_BspTexture* faceTex = surface->faceTexture->texture;
    
    X_Texture tex;
    tex.texels = faceTex->mipTexels[mipLevel];
    tex.w = faceTex->w >> mipLevel;
    tex.h = faceTex->h >> mipLevel;
    
    int uMask = tex.w - 1;
    int vMask = tex.h - 1;
    
    int minX = surface->textureMinCoord.x >> 16;
    int minY = surface->textureMinCoord.y >> 16;
    
    while(minX < 0) minX += tex.w;
    while(minY < 0) minY += tex.h;
    
    _Bool canUseWrapMask = 1;//is_power_of_2(w) && is_power_of_2(h);
    
    if(!is_power_of_2(tex.w))
        uMask = 255;
    
    if(!is_power_of_2(tex.h))
        vMask = 255;
    
    if(!renderer->enableLighting)
    {
        for(int i = 0; i < h; ++i)
        {
            for(int j = 0; j < w; ++j)
            {
                if(canUseWrapMask)
                    texels[i * w + j] = x_texture_get_texel(&tex, (j + minX) & uMask, (i + minY) & vMask);
                else
                    texels[i * w + j] = x_texture_get_texel(&tex, (j + minX) % w, (i + minY) % h);
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
                    
                    X_Color texel;
                    
                    if(canUseWrapMask)
                        texel = x_texture_get_texel(&tex, (x + minX) & uMask, (y + minY) & vMask);
                    else
                        texel = x_texture_get_texel(&tex, (x + minX) % w, (y + minY) % h);
                    
                    int intensity = X_MIN(63, (left + dRow * d) >> (16 + 2));
                    
                    texels[y * w + x] = colorMap[texel][intensity];
                }
            }
        }
    }

    printf("Build surface %d\n", surface->id);
    
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


