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

#define X_LIGHTMAP_MAX_SIZE 34

typedef struct X_SurfaceBuilderBlock
{
    int startX;
    int startY;
    int blockSize;
    
    x_fp16x16 topLeftIntensity;
    x_fp16x16 topRightIntensity;
    x_fp16x16 dLeft;
    x_fp16x16 dRight;
} X_SurfaceBuilderBlock;

typedef struct X_SurfaceBuilder
{
    X_BspSurface* bspSurface;
    X_Texture surface;
    
    int combinedLightmap[X_LIGHTMAP_MAX_SIZE * X_LIGHTMAP_MAX_SIZE];
    X_Vec2 lightmapSize;
    int lightmapTotalLumels;
    
    X_Texture texture;
    X_Vec2 textureOffset;
    X_Vec2 textureMask;
    
    X_Renderer* renderer;
    
    int mipLevel;
    
    X_SurfaceBuilderBlock block;
    
    X_Light* currentLight;
} X_SurfaceBuilder;

// TODO: should this be moved into utils?
static _Bool is_power_of_2(int val)
{
    return val != 0 && (val & (val - 1)) == 0;
}

static void clear_to_ambient_light(int* lightmap, int lightmapSize)
{
    for(int i = 0; i < lightmapSize; ++i)
        lightmap[i] = 0;
}

static void x_surfacebuilder_combine_lightmaps(X_SurfaceBuilder* builder)
{
    const unsigned char END_OF_LIGHTMAPS = 255;
    unsigned char* lumels = builder->bspSurface->lightmapData;
    
    for(int i = 0; i < X_BSPSURFACE_MAX_LIGHTMAPS; ++i)
    {
        if(builder->bspSurface->lightmapStyles[i] == END_OF_LIGHTMAPS)
            break;
        
        for(int j = 0; j < builder->lightmapTotalLumels; ++j)
            builder->combinedLightmap[j] += lumels[j];
        
        // Lightmaps are stored consecutively in memory
        lumels += builder->lightmapTotalLumels;
    }
}

static void x_surfacebuilder_calculate_texture_offset(X_SurfaceBuilder* builder)
{
    // Guarantee a positive coordinate by adding 64k x the size of the texture
    builder->textureOffset = x_vec2_make
    (
        ((builder->bspSurface->textureMinCoord.x >> 16) + (builder->texture.w << 16)) % builder->texture.w,
        ((builder->bspSurface->textureMinCoord.y >> 16) + (builder->texture.h << 16)) % builder->texture.h
    );
}

static X_Vec2 x_texture_get_repeat_mask(X_Texture* texture)
{
    return x_vec2_make
    (
        is_power_of_2(texture->w) ? texture->w - 1 : 255, 
        is_power_of_2(texture->h) ? texture->h - 1 : 255
    );
}

static void x_surfacebuilder_calculate_surface_size(X_SurfaceBuilder* builder)
{
    builder->surface.w = builder->bspSurface->textureExtent.x >> (builder->mipLevel + 16);
    builder->surface.h = builder->bspSurface->textureExtent.y >> (builder->mipLevel + 16);
}

static void x_surfacebuilder_calculate_lightmap_size(X_SurfaceBuilder* builder)
{
    builder->lightmapSize = x_vec2_make
    (
        (builder->bspSurface->textureExtent.x >> (16 + 4)) + 1,
        (builder->bspSurface->textureExtent.y >> (16 + 4)) + 1
    );
    
    builder->lightmapTotalLumels = builder->lightmapSize.x * builder->lightmapSize.y;
}

static X_Color x_surfacebuilder_get_texture_texel(X_SurfaceBuilder* builder, int surfaceX, int surfaceY)
{
    int textureX = ((surfaceX ) + builder->textureOffset.x) & builder->textureMask.x;
    int textureY = ((surfaceY ) + builder->textureOffset.y) & builder->textureMask.y;
    
    return x_texture_get_texel(&builder->texture, textureX, textureY);
}

static void x_surfacebuilder_build_without_lighting(X_SurfaceBuilder* builder)
{
    for(int i = 0; i < builder->surface.h; ++i)
    {
        for(int j = 0; j < builder->surface.w; ++j)
        {
            builder->surface.texels[i * builder->surface.w + j] = x_surfacebuilder_get_texture_texel(builder, j, i);
        }
    }
}

static x_fp16x16 lightmap_get_lumel(int* lightmap, X_Vec2 lightmapSize, int x, int y)
{
    return lightmap[y * lightmapSize.x + x] << 16;
}

static x_fp16x16 x_surfacebuilderblock_get_intensity_at_offset(X_SurfaceBuilderBlock* block, int offsetX, int offsetY, int mipLevel)
{
    x_fp16x16 left = block->topLeftIntensity + offsetY * block->dLeft;
    x_fp16x16 right = block->topRightIntensity + offsetY * block->dRight;
    x_fp16x16 dRow = (right - left) >> (4 - mipLevel);
    
    return left + offsetX * dRow;
}

static void x_surfacebuilder_build_16x16_block(X_SurfaceBuilder* builder)
{
    X_SurfaceBuilderBlock* block = &builder->block;
    
    for(int i = 0; i < block->blockSize; ++i)
    {
        for(int j = 0; j < block->blockSize; ++j)
        {
            int x = block->startX + j;
            int y = block->startY + i;
            
            X_Color texel = x_surfacebuilder_get_texture_texel(builder, x, y);
            x_fp16x16 intensity = x_surfacebuilderblock_get_intensity_at_offset(block, j, i, builder->mipLevel);
            
            builder->surface.texels[y * builder->surface.w + x] = x_renderer_get_shaded_color(builder->renderer, texel, intensity >> (16 + 2));
        }
    }
}

static void x_surfacebuilder_init_block(X_SurfaceBuilder* builder, int blockX, int blockY)
{
    int topLeft =       lightmap_get_lumel(builder->combinedLightmap, builder->lightmapSize, blockX, blockY);
    int topRight =      lightmap_get_lumel(builder->combinedLightmap, builder->lightmapSize, blockX + 1, blockY);
    int bottomLeft =    lightmap_get_lumel(builder->combinedLightmap, builder->lightmapSize, blockX, blockY + 1);
    int bottomRight =   lightmap_get_lumel(builder->combinedLightmap, builder->lightmapSize, blockX + 1, blockY + 1);
    
    X_SurfaceBuilderBlock* block = &builder->block;
    block->topLeftIntensity = topLeft;
    block->topRightIntensity = topRight;
    block->dLeft = (bottomLeft - topLeft) >> (4 - builder->mipLevel);
    block->dRight = (bottomRight - topRight) >> (4 - builder->mipLevel);
    block->blockSize = 16 >> (builder->mipLevel);
    block->startX = blockX * (16 >> builder->mipLevel);
    block->startY = blockY * (16 >> builder->mipLevel);
}

static void x_surfacebuilder_build_from_combined_lightmap(X_SurfaceBuilder* builder)
{
    for(int i = 0; i < builder->lightmapSize.y - 1; ++i)
    {
        for(int j = 0; j < builder->lightmapSize.x - 1; ++j)
        {
            x_surfacebuilder_init_block(builder, j, i);
            x_surfacebuilder_build_16x16_block(builder);
        }
    }
}

static void x_surfacebuilder_surface_point_closest_to_light(X_SurfaceBuilder* builder, X_Vec3* dest, int* distDest)
{
    X_Vec3 lightPos = x_vec3_fp16x16_to_vec3(&builder->currentLight->position);
    X_Plane* plane = &builder->bspSurface->plane->plane;
    int dist = x_fp16x16_to_int(x_plane_point_distance(plane, &lightPos));
    
    dest->x = lightPos.x - x_fp16x16_to_int(plane->normal.x * dist);
    dest->y = lightPos.y - x_fp16x16_to_int(plane->normal.y * dist);
    dest->z = lightPos.z - x_fp16x16_to_int(plane->normal.z * dist);
    
    *distDest = abs(dist);
}

static void x_surfacebuilder_apply_dynamic_light(X_SurfaceBuilder* builder)
{
    int lightDistToPlane;
    X_Vec3 closestPoint;
    x_surfacebuilder_surface_point_closest_to_light(builder, &closestPoint, &lightDistToPlane);
    
    // We use linear falloff
    x_fp24x8 intensityAtClosestPoint = builder->currentLight->intensity - lightDistToPlane;
    
    X_BspFaceTexture* faceTexture = builder->bspSurface->faceTexture;
    X_Vec2 closestIn2D = x_vec2_make
    (
        x_fp16x16_to_int(x_vec3_dot(&closestPoint, &faceTexture->uOrientation) + faceTexture->uOffset - builder->bspSurface->textureMinCoord.x),
        x_fp16x16_to_int(x_vec3_dot(&closestPoint, &faceTexture->vOrientation) + faceTexture->vOffset - builder->bspSurface->textureMinCoord.y)
    );
    
    for(int i = 0; i < builder->lightmapSize.y; ++i)
    {
        int vDist = abs(closestIn2D.y - i * 16);
        
        for(int j = 0; j < builder->lightmapSize.x; ++j)
        {
            int uDist = abs(closestIn2D.x - j * 16);
            int approxDist = (uDist > vDist ? uDist + (vDist >> 1) : vDist + (uDist >> 1));
            
            builder->combinedLightmap[i * builder->lightmapSize.x + j] += X_MAX(0, intensityAtClosestPoint - approxDist);
        }
    }
}

static void x_surfacebuilder_clamp_lightmap(X_SurfaceBuilder* builder)
{
    for(int i = 0; i < builder->lightmapTotalLumels; ++i)
        builder->combinedLightmap[i] = X_MIN(255, builder->combinedLightmap[i]);
}

static void x_surfacebuilder_build_with_lighting(X_SurfaceBuilder* builder)
{
    clear_to_ambient_light(builder->combinedLightmap, builder->lightmapTotalLumels);
    x_surfacebuilder_combine_lightmaps(builder);
    
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        builder->currentLight = builder->renderer->dynamicLights + i;
        
        if(x_light_is_enabled(builder->currentLight))
        {
            x_surfacebuilder_apply_dynamic_light(builder);
        }
    }
    
    x_surfacebuilder_clamp_lightmap(builder);
    
    x_surfacebuilder_build_from_combined_lightmap(builder);
}

static void x_surfacebuilder_init(X_SurfaceBuilder* builder, X_BspSurface* surface, int mipLevel, X_Renderer* renderer)
{
    builder->renderer = renderer;
    builder->mipLevel = mipLevel;
    builder->bspSurface = surface;
    
    x_surfacebuilder_calculate_surface_size(builder);
    x_surfacebuilder_calculate_lightmap_size(builder);
    
    int totalTexels = builder->surface.w * builder->surface.h;
    
    if(!x_cachentry_is_in_cache(surface->cachedSurfaces + mipLevel))
        x_cache_alloc(&renderer->surfaceCache, totalTexels, surface->cachedSurfaces + mipLevel);
    
    builder->surface.texels = x_cache_get_cached_data(&renderer->surfaceCache, surface->cachedSurfaces + mipLevel);
    
    X_BspTexture* faceTex = surface->faceTexture->texture;
    builder->texture.texels = faceTex->mipTexels[mipLevel];
    builder->texture.w = faceTex->w >> mipLevel;
    builder->texture.h = faceTex->h >> mipLevel;
    builder->textureMask = x_texture_get_repeat_mask(&builder->texture);
    
    x_surfacebuilder_calculate_texture_offset(builder);
}

static void __attribute__((hot)) x_bspsurface_rebuild(X_BspSurface* surface, int mipLevel, X_Renderer* renderer)
{
    X_SurfaceBuilder builder;
    x_surfacebuilder_init(&builder, surface, mipLevel, renderer);
    
    if(renderer->enableLighting)
        x_surfacebuilder_build_with_lighting(&builder);
    else
        x_surfacebuilder_build_without_lighting(&builder);
}

static _Bool x_bspsurface_need_to_rebuild_because_lights_changed(X_BspSurface* surface, X_Renderer* renderer)
{
    return surface->lastLightUpdateFrame == renderer->currentFrame &&
        (surface->lightsTouchingSurface & renderer->dynamicLightsNeedingUpdated) != 0;
}

void x_bspsurface_get_surface_texture_for_mip_level(X_BspSurface* surface, int mipLevel, X_Renderer* renderer, X_Texture* dest)
{
    if(!x_cachentry_is_in_cache(surface->cachedSurfaces + mipLevel) || x_bspsurface_need_to_rebuild_because_lights_changed(surface, renderer))
        x_bspsurface_rebuild(surface, mipLevel, renderer);
    
    dest->w = surface->textureExtent.x >> (mipLevel + 16);
    dest->h = surface->textureExtent.y >> (mipLevel + 16);
    dest->texels = x_cache_get_cached_data(&renderer->surfaceCache, surface->cachedSurfaces + mipLevel);
}

