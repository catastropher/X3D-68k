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

#include "X_span.h"
#include "level/X_BspLevel.h"
#include "math/X_Mat4x4.h"
#include "X_RenderContext.h"
#include "X_activeedge.h"
#include "X_Renderer.h"

#define SHIFTUP 8

static void x_ae_surfacerendercontext_init_sdivz(X_AE_SurfaceRenderContext* context)
{
    X_Vec3 sAxis;
    X_BspFaceTexture* tex = context->faceTexture;
    x_mat4x4_rotate_normal(context->renderContext->viewMatrix, &tex->uOrientation, &sAxis);
    
    X_AE_TextureVar* sDivZ = &context->sDivZ;
    sDivZ->uOrientationStep = ((sAxis.x << SHIFTUP) / context->viewport->distToNearPlane) >> context->mipLevel;
    sDivZ->vOrientationStep = ((sAxis.y << SHIFTUP) / context->viewport->distToNearPlane) >> context->mipLevel;
    
    //x_vec3_fp16x16_print(&tex->vOrientation, "V");
    
    // TODO: move these into viewport struct
    int centerX = context->viewport->screenPos.x + context->viewport->w / 2;
    int centerY = context->viewport->screenPos.y + context->viewport->h / 2;
    
    sDivZ->origin = ((sAxis.z << SHIFTUP) >> context->mipLevel) - centerX * sDivZ->uOrientationStep - centerY * sDivZ->vOrientationStep;
    
    X_Vec3 transormed;
    X_Vec3 pos = x_vec3_neg(&x_bsplevel_get_level_model(context->renderContext->level)->origin);
    x_mat4x4_transform_vec3(context->renderContext->viewMatrix, &pos, &transormed);
    
    transormed.x >>= context->mipLevel;
    transormed.y >>= context->mipLevel;
    transormed.z >>= context->mipLevel;
    
    sDivZ->adjust = -x_vec3_dot(&sAxis, &transormed) /*- ((context->surface->bspSurface->textureMinCoord.x) >> context->mipLevel)*/ +
        ((context->faceTexture->uOffset) >> context->mipLevel);
}

static void x_ae_surfacerendercontext_init_tdivz(X_AE_SurfaceRenderContext* context)
{
    X_Vec3 tAxis;
    X_BspFaceTexture* tex = context->faceTexture;
    x_mat4x4_rotate_normal(context->renderContext->viewMatrix, &tex->vOrientation, &tAxis);
    
    X_AE_TextureVar* tDivZ = &context->tDivZ;
    tDivZ->uOrientationStep = ((tAxis.x << SHIFTUP) / context->viewport->distToNearPlane) >> context->mipLevel;
    tDivZ->vOrientationStep = ((tAxis.y << SHIFTUP) / context->viewport->distToNearPlane) >> context->mipLevel;
    
    // TODO: move these into viewport struct
    int centerX = context->viewport->screenPos.x + context->viewport->w / 2;
    int centerY = context->viewport->screenPos.y + context->viewport->h / 2;
    
    tDivZ->origin = ((tAxis.z << SHIFTUP) >> context->mipLevel) - centerX * tDivZ->uOrientationStep - centerY * tDivZ->vOrientationStep;
    
    X_Vec3 transormed;
    X_Vec3 pos = x_bsplevel_get_level_model(context->renderContext->level)->origin;
    x_mat4x4_transform_vec3(context->renderContext->viewMatrix, &pos, &transormed);
    
    transormed.x >>= context->mipLevel;
    transormed.y >>= context->mipLevel;
    transormed.z >>= context->mipLevel;
    
    tDivZ->adjust = -x_vec3_dot(&tAxis, &transormed) /*- ((context->surface->bspSurface->textureMinCoord.y) >> context->mipLevel)*/ + 
        ((context->faceTexture->vOffset) >> context->mipLevel);        
}

void x_ae_surfacerendercontext_init(X_AE_SurfaceRenderContext* context, X_AE_Surface* surface, X_RenderContext* renderContext, int mipLevel)
{
    context->surface = surface;
    context->faceTexture = context->surface->bspSurface->faceTexture;
    context->renderContext = renderContext;
    context->mipLevel = mipLevel;
    context->viewport = &renderContext->cam->viewport;
    
    x_ae_surfacerendercontext_init_sdivz(context);
    x_ae_surfacerendercontext_init_tdivz(context);
    
    X_BspLevel* level = context->renderContext->level;
    x_bsplevel_get_texture(level, context->faceTexture->texture - level->textures, context->mipLevel, &context->faceTex);
    
    context->uMask = context->faceTex.w - 1;        // Must be powers of 2 for this to work!
    context->vMask = context->faceTex.h - 1;
}

static inline x_fp16x16 calculate_u_div_z(const X_AE_SurfaceRenderContext* context, int x, int y)
{
    return  ((2 * x + 1) * context->sDivZ.uOrientationStep + (2 * y + 1)  * context->sDivZ.vOrientationStep) / 2 + context->sDivZ.origin;
}

static inline x_fp16x16 calculate_v_div_z(const X_AE_SurfaceRenderContext* context, int x, int y)
{
    return ((2 * x + 1) * context->tDivZ.uOrientationStep + (2 * y + 1)  * context->tDivZ.vOrientationStep) / 2 + context->tDivZ.origin;
}

float g_zbuf[480][640];

static inline void calculate_u_and_v_at_screen_point(const X_AE_SurfaceRenderContext* context, int x, int y, x_fp16x16* u, x_fp16x16* v)
{
    x_fp16x16 uDivZ = calculate_u_div_z(context, x, y);
    x_fp16x16 vDivZ = calculate_v_div_z(context, x, y);
    x_fp2x30 invZ = (x_ae_surface_calculate_inverse_z_at_screen_point(context->surface, x, y)) >> 13;
    
    if(invZ == 0)
        return;
    
    int z = (1 << 17) / invZ;
        
    *u = ((((long long)uDivZ * z) >> SHIFTUP) + context->sDivZ.adjust);
    *v = ((((long long)vDivZ * z) >> SHIFTUP) + context->tDivZ.adjust);
}

//#define CLAMP

static inline void draw_texel(X_AE_SurfaceRenderContext* context, x_fp16x16* u, x_fp16x16* v, x_fp16x16 du, x_fp16x16 dv, X_Color* pixel)
{
    int uu = (*u + context->surface->bspSurface->textureMinCoord.x) >> 16;
    int vv = (*v + context->surface->bspSurface->textureMinCoord.y) >> 16;
    
    uu = uu & context->uMask;
    vv = vv & context->vMask;
    
    *pixel = x_texture_get_texel(&context->faceTex, uu, vv);
    *u += du;
    *v += dv;
}

static inline X_Color get_texel(const X_AE_SurfaceRenderContext* context, x_fp16x16 u, x_fp16x16 v)
{
    int uu = (u >> 16) & context->uMask;
    int vv = (v >> 16) & context->vMask;
    
    return context->faceTex.texels[vv * context->faceTex.w + uu];
}

#define WRITE_TEXELS_SHORT()            \
    lo = get_texel(context, *u, *v);    \
    *u += du;                           \
    *v += dv;                           \
    hi = get_texel(context, *u, *v);    \
    *((unsigned short*)pixels) = ((unsigned short)hi << 8) | lo;

static inline void draw_unaligned_group(const X_AE_SurfaceRenderContext* context, X_Color* pixels, x_fp16x16 count, x_fp16x16* restrict u, x_fp16x16* restrict v, int x, int y)
{
    if(count == 0)
        return;
    
    x_fp16x16 nextU, nextV;
    calculate_u_and_v_at_screen_point(context, x + count, y, &nextU, &nextV);
    
    x_fp16x16 du, dv;
    
    X_Color hi, lo;
    
    switch(count)
    {
        case 1:
            *pixels = get_texel(context, *u, *v);
            break;
            
        case 2:
            du = (nextU - *u) >> 1;
            dv = (nextV - *v) >> 1;
            WRITE_TEXELS_SHORT();
            break;
            
        case 3:
            du = (nextU - *u) / 3;
            dv = (nextV - *v) / 3;
            *pixels++ = get_texel(context, *u, *v);
            *u += du;
            *v += dv;
            WRITE_TEXELS_SHORT();
            break;
    }
    
    *u = nextU;
    *v = nextV;
}

static inline void draw_aligned_group(const X_AE_SurfaceRenderContext* context, X_Color* pixels, x_fp16x16 du, x_fp16x16 dv, x_fp16x16* restrict u, x_fp16x16* restrict v)
{
    unsigned int a, b, c, d;
    
    a = get_texel(context, *u, *v);     *u += du;       *v += dv;
    b = get_texel(context, *u, *v);     *u += du;       *v += dv;
    c = get_texel(context, *u, *v);     *u += du;       *v += dv;
    d = get_texel(context, *u, *v);     *u += du;       *v += dv;
    
    *((unsigned int*)pixels) = a | (b << 8) | (c << 16) | (d << 24);
}

static inline void draw_aligned_16_group(const X_AE_SurfaceRenderContext* context, X_Color* pixels, x_fp16x16* restrict u, x_fp16x16* restrict v, int x, int y)
{
    const int GROUP_SIZE = 16;
    x_fp16x16 nextU, nextV;
    calculate_u_and_v_at_screen_point(context, x + GROUP_SIZE, y, &nextU, &nextV);
    
    x_fp16x16 du = (nextU - *u) >> 4;
    x_fp16x16 dv = (nextV - *v) >> 4;
    
    draw_aligned_group(context, pixels, du, dv, u, v);  pixels += 4;
    draw_aligned_group(context, pixels, du, dv, u, v);  pixels += 4;
    draw_aligned_group(context, pixels, du, dv, u, v);  pixels += 4;
    draw_aligned_group(context, pixels, du, dv, u, v);  pixels += 4;
}

static inline void draw_small_group(const X_AE_SurfaceRenderContext* context, X_Color* pixels, x_fp16x16 count, x_fp16x16* restrict u, x_fp16x16* restrict v, int x, int y)
{
    x_fp16x16 nextU, nextV;
    calculate_u_and_v_at_screen_point(context, x + count, y, &nextU, &nextV);
    
    x_fp16x16 du = (nextU - *u) / count;
    x_fp16x16 dv = (nextV - *v) / count;
    
    for(int i = 0; i < count; ++i)
    {
        *pixels++ = get_texel(context, *u, *v);
        *u += du;
        *v += dv;
    }
}

static inline void __attribute__((hot)) x_ae_surfacerendercontext_render_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span)
{
    X_Texture* screenTex = &context->renderContext->screen->canvas.tex;
    X_Color* scanline = screenTex->texels + span->y * screenTex->w;
    X_Color* pixels = scanline + span->x1;
    
    x_fp16x16 u, v;
    calculate_u_and_v_at_screen_point(context, span->x1, span->y, &u, &v);
    
    int count = span->x2 - span->x1;
    if(count == 0)
        return;
    
    if(count < 4)
    {
        draw_small_group(context, pixels, count, &u, &v, span->x1, span->y);
        return;
    }
    
    X_Color* pixelsEnd = pixels + count;
    
    // First get us to a pixel address that's a multiple of 4
    int unalignedCount = 4 - (span->x1 & 3);
    if(unalignedCount != 4)     // 4 means we're already a multiple of 4
    {
        draw_unaligned_group(context, pixels, unalignedCount, &u, &v, span->x1, span->y);
        pixels += unalignedCount;
    }
    
    // Now, draw groups of 16 pixels that are all nicely aligned, so we can pack 4 texels into a single write
    while(pixels + 16 < pixelsEnd)
    {
        draw_aligned_16_group(context, pixels, &u, &v, pixels - scanline, span->y);
        pixels += 16;
    }
    
    int pixelsLeft = pixelsEnd - pixels;
    x_fp16x16 nextU, nextV;
    calculate_u_and_v_at_screen_point(context, pixels - scanline + pixelsLeft, span->y, &nextU, &nextV);
    
    x_fp16x16 du = (nextU - u) / pixelsLeft;
    x_fp16x16 dv = (nextV - v) / pixelsLeft;
    
    // Draw as many aligned groups of 4 as possible (< 16 texels left)
    while(pixels + 4 < pixelsEnd)
    {
        draw_aligned_group(context, pixels, du, dv, &u, &v);
        pixels += 4;
    }
    
    // Draw whatever is left
    while(pixels < pixelsEnd)
    {
        *pixels++ = get_texel(context, u, v);
        u += du;
        v += dv;
    }
}

void x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context)
{
    if(((context->renderContext->renderer->renderMode) & 1) == 0)
        return;
    
    for(int i = 0; i < context->surface->totalSpans; ++i)
    {
        x_ae_surfacerendercontext_render_span(context, context->surface->spans + i);
    }
}



