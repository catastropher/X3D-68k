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
    
    sDivZ->adjust = -x_vec3_dot(&sAxis, &transormed) - ((context->surface->bspSurface->textureMinCoord.x) >> context->mipLevel) +
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
    
    tDivZ->adjust = -x_vec3_dot(&tAxis, &transormed) - ((context->surface->bspSurface->textureMinCoord.y) >> context->mipLevel) + 
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

static inline x_fp16x16 calculate_u_div_z(X_AE_SurfaceRenderContext* context, int x, int y)
{
    return  ((2 * x + 1) * context->sDivZ.uOrientationStep + (2 * y + 1)  * context->sDivZ.vOrientationStep) / 2 + context->sDivZ.origin;
}

static inline x_fp16x16 calculate_v_div_z(X_AE_SurfaceRenderContext* context, int x, int y)
{
    return ((2 * x + 1) * context->tDivZ.uOrientationStep + (2 * y + 1)  * context->tDivZ.vOrientationStep) / 2 + context->tDivZ.origin;
}

float g_zbuf[480][640];

static inline void calculate_u_and_v_at_screen_point(X_AE_SurfaceRenderContext* context, int x, int y, x_fp16x16* u, x_fp16x16* v)
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

static inline void __attribute__((hot)) x_ae_surfacerendercontext_render_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span)
{
    X_Texture* screenTex = &context->renderContext->screen->canvas.tex;
    X_Color* scanline = screenTex->texels + span->y * screenTex->w;
    
    int count = span->x2 - span->x1;
    if(count == 0)
        return;
    
    x_fp16x16 prevU;
    x_fp16x16 prevV;
    
    int y = span->y;
    
    calculate_u_and_v_at_screen_point(context, span->x1, y, &prevU, &prevV);
    
    X_Color* pixel = scanline + span->x1;
    X_Color* pixelEnd = scanline + span->x2;
    
    int SHIFT = 4;
    int COUNT = (1 << SHIFT);
    
    x_fp16x16 nextU;
    x_fp16x16 nextV;
    
    x_fp16x16 u;
    x_fp16x16 v;
    
    x_fp16x16 du;
    x_fp16x16 dv;
    
    X_Color* end;
        
    // Clamp to the texture bounds
    X_Vec2 extent = context->surface->bspSurface->textureExtent;
    
    X_Vec2 maxs = extent;//x_vec2_make(mins.x + extent.x, mins.y + extent.y);
    
    // Clamp
#ifdef CLAMP
    if(prevU < 0)
        prevU = 0;
    else if(prevU > maxs.x)
        prevU = maxs.x;
    
    if(prevV < 0)
        prevV = 0;
    else if(prevV > maxs.y)
        prevV = maxs.y;
#endif
    
    const int ROUNDOFF_ERROR_GUARD = COUNT;
    
    if(pixelEnd - pixel < COUNT)
        goto last_group;
    
    do
    {
        calculate_u_and_v_at_screen_point(context, pixel - scanline + COUNT, y, &nextU, &nextV);
        
        if(nextU > maxs.x)
            nextU = maxs.x;
        
        if(nextV > maxs.y)
            nextV = maxs.y;
        
#ifdef CLAMP
        if(nextU > maxs.x)
            nextU = maxs.x;
        else if(nextU < ROUNDOFF_ERROR_GUARD)
            nextU = ROUNDOFF_ERROR_GUARD;
        
        if(nextV > maxs.y)
            nextV = maxs.y;
        else if(nextV < ROUNDOFF_ERROR_GUARD)
            nextV = ROUNDOFF_ERROR_GUARD;
#endif
        
        u = prevU;
        v = prevV;
        
        du = (nextU - prevU) >> SHIFT;
        dv = (nextV - prevV) >> SHIFT;
        
        end = pixel + COUNT;
        if(end > pixelEnd)
            end = pixelEnd;
        
draw_group:
        
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        
        prevU = nextU;
        prevV = nextV;
    } while(pixel + COUNT < pixelEnd);
    
    if(pixel != pixelEnd)
    {
last_group:
        end = pixelEnd;
        calculate_u_and_v_at_screen_point(context, span->x2 - 1, y, &nextU, &nextV);
      
#ifdef CLAMP
        if(nextU > maxs.x)
            nextU = maxs.x;
        else if(nextU < ROUNDOFF_ERROR_GUARD)
            nextU = ROUNDOFF_ERROR_GUARD;
        
        if(nextV > maxs.y)
            nextV = maxs.y;
        else if(nextV < ROUNDOFF_ERROR_GUARD)
            nextV = ROUNDOFF_ERROR_GUARD;
#endif
        
        
        int count = pixelEnd - pixel;
        du = (nextU - prevU) / count;
        dv = (nextV - prevV) / count;
        
        u = prevU;
        v = prevV;
        
        switch(COUNT - count)
        {
            case 0:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 1:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 2:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 3:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 4:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 5:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 6:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 7:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 8:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 9:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 10:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 11:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 12:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 13:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 14:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
            case 15:     draw_texel(context, &u, &v, du, dv, pixel);     ++pixel;
        }
        
    }
}

void x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context)
{
    for(int i = 0; i < context->surface->totalSpans; ++i)
    {
        x_ae_surfacerendercontext_render_span(context, context->surface->spans + i);
    }
}



