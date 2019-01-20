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
#include "X_Surface.h"
#include "object/X_CameraObject.h"

// Scales a value down based on the current mip map level
static inline int mip_adjust(int val, int mipLevel)
{
    return val >> mipLevel;
}

static inline fp mip_adjust(fp val, int mipLevel)
{
    return fp(val.toFp16x16() >> mipLevel);
}

static inline void rotate_vector_into_eye_space(X_AE_SurfaceRenderContext* context, Vec3fp* vecToRotate, Vec3fp* dest)
{
    *dest = context->renderContext->viewMatrix->transformNormal(*vecToRotate);
}

static inline void calculate_uv_orientation_steps_in_screen_space(X_AE_TextureVar* var, X_AE_SurfaceRenderContext* context, Vec3fp* orientationInEyeSpace)
{
    var->uOrientationStep = mip_adjust(orientationInEyeSpace->x / context->viewport->distToNearPlane, context->mipLevel).toFp16x16();
    var->vOrientationStep = mip_adjust(orientationInEyeSpace->y / context->viewport->distToNearPlane, context->mipLevel).toFp16x16();
}

static inline void calculate_uv_origin_relative_to_screen_top_left(X_AE_TextureVar* var, X_AE_SurfaceRenderContext* context, Vec3fp* orientationInEyeSpace)
{
    int centerX = context->viewport->screenPos.x + context->viewport->w / 2;
    int centerY = context->viewport->screenPos.y + context->viewport->h / 2;
    
    var->origin = mip_adjust(orientationInEyeSpace->z, context->mipLevel).toFp16x16() -
        centerX * var->uOrientationStep -
        centerY * var->vOrientationStep;
}

static inline void calculate_texture_adjustment(X_AE_TextureVar* var, X_AE_SurfaceRenderContext* context, Vec3fp* orientationInEyeSpace, int minTexCoord, int texOffset)
{
    Vec3fp inverseModelPos = *context->surface->modelOrigin;
    
    Vec3fp inverseModelPosInEyeSpace = context->renderContext->viewMatrix->transform(inverseModelPos);
    
    int mipLevel = context->mipLevel;
    inverseModelPosInEyeSpace = Vec3fp(
        inverseModelPosInEyeSpace.x >> mipLevel,
        inverseModelPosInEyeSpace.y >> mipLevel,
        inverseModelPosInEyeSpace.z >> mipLevel);
    
    var->adjust = -orientationInEyeSpace->dot(inverseModelPosInEyeSpace).toFp16x16() -
        mip_adjust(minTexCoord, mipLevel) +
        mip_adjust(texOffset, mipLevel);
}

static void x_ae_texturevar_init(X_AE_TextureVar* var, X_AE_SurfaceRenderContext* context, Vec3fp* orientationAxis, int minTexCoord, int texOffset)
{
    Vec3fp orientationInEyeSpace;
    rotate_vector_into_eye_space(context, orientationAxis, &orientationInEyeSpace);
    
    calculate_uv_orientation_steps_in_screen_space(var, context, &orientationInEyeSpace);
    calculate_uv_origin_relative_to_screen_top_left(var, context, &orientationInEyeSpace);
    calculate_texture_adjustment(var, context, &orientationInEyeSpace, minTexCoord, texOffset);
}

const x_fp16x16 recip_tab[32] = 
{
    0,
    X_FP16x16_ONE / 1,
    X_FP16x16_ONE / 2,
    X_FP16x16_ONE / 3,
    X_FP16x16_ONE / 4,
    X_FP16x16_ONE / 5,
    X_FP16x16_ONE / 6,
    X_FP16x16_ONE / 7,
    X_FP16x16_ONE / 8,
    X_FP16x16_ONE / 9,
    X_FP16x16_ONE / 10,
    X_FP16x16_ONE / 11,
    X_FP16x16_ONE / 12,
    X_FP16x16_ONE / 13,
    X_FP16x16_ONE / 14,
    X_FP16x16_ONE / 15,
    X_FP16x16_ONE / 16,
    X_FP16x16_ONE / 17,
    X_FP16x16_ONE / 18,
    X_FP16x16_ONE / 19,
    X_FP16x16_ONE / 20,
    X_FP16x16_ONE / 21,
    X_FP16x16_ONE / 22,
    X_FP16x16_ONE / 23,
    X_FP16x16_ONE / 24,
    X_FP16x16_ONE / 25,
    X_FP16x16_ONE / 26,
    X_FP16x16_ONE / 27,
    X_FP16x16_ONE / 28,
    X_FP16x16_ONE / 29,
    X_FP16x16_ONE / 30,
    X_FP16x16_ONE / 31,
};

static inline void setup_inv_z_step(X_AE_SurfaceRenderContext* context)
{
    X_AE_Surface* surface = context->surface;
    context->invZStepX = surface->zInverseXStep;
    context->invZStepY = surface->zInverseYStep;
    context->invZOrigin = surface->zInverseOrigin;
    
    context->invZStepXNeg = -context->invZStepX;
    context->uStepXNeg = -context->uStepX;
    context->vStepXNeg = -context->vStepX;
}

static inline void setup_u_step(X_AE_SurfaceRenderContext* context)
{
    context->uStepX = context->sDivZ.uOrientationStep;
    context->uStepY = context->sDivZ.vOrientationStep;
    context->uOrigin = context->sDivZ.origin;
    context->uAdjust = context->sDivZ.adjust;
}

static inline void setup_v_step(X_AE_SurfaceRenderContext* context)
{
    context->vStepX = context->tDivZ.uOrientationStep;
    context->vStepY = context->tDivZ.vOrientationStep;
    context->vStepOrigin = context->tDivZ.origin;
    context->vAdjust = context->tDivZ.adjust;
}

static inline void setup_surface(X_AE_SurfaceRenderContext* context)
{
    X_Vec2 surfaceSize = context->surface->bspSurface->textureExtent;
    context->surfaceW = mip_adjust(surfaceSize.x, context->mipLevel) - X_FP16x16_ONE;
    context->surfaceH = mip_adjust(surfaceSize.y, context->mipLevel) - X_FP16x16_ONE;
    context->texW = context->surfaceTexture.getW();
    
    context->surfaceTexels = context->surfaceTexture.getTexels();
}

static inline void setup_recip_tab(X_AE_SurfaceRenderContext* context)
{
    context->recipTab = recip_tab;
}

static inline void setup_screen(X_AE_SurfaceRenderContext* context)
{
    context->screen = context->renderContext->canvas->getTexels();
}

void x_ae_surfacerendercontext_setup_constants(X_AE_SurfaceRenderContext* context)
{    
    setup_u_step(context);
    setup_v_step(context);
    setup_inv_z_step(context);
    setup_surface(context);
    setup_recip_tab(context);
    setup_screen(context);
}

void x_ae_surfacerendercontext_init(X_AE_SurfaceRenderContext* context, X_AE_Surface* surface, X_RenderContext* renderContext, int mipLevel)
{
    context->surface = surface;
    context->faceTexture = context->surface->bspSurface->faceTexture;
    context->renderContext = renderContext;
    context->mipLevel = renderContext->cam->viewport.closestMipLevelForZ(fp(surface->closestZ));
    context->viewport = &renderContext->cam->viewport;

    if(surface->flags.hasFlag(SURFACE_FILL_SOLID))
    {
        return;
    }
    
    X_BspFaceTexture* tex = context->faceTexture;
    X_BspSurface* bspSurface = context->surface->bspSurface;
    
    x_ae_texturevar_init(&context->sDivZ, context, &tex->uOrientation, bspSurface->textureMinCoord.x, tex->uOffset);
    x_ae_texturevar_init(&context->tDivZ, context, &tex->vOrientation, bspSurface->textureMinCoord.y, tex->vOffset);
    
    x_bspsurface_get_surface_texture_for_mip_level(surface->bspSurface, context->mipLevel, renderContext->renderer, &context->surfaceTexture);
    x_ae_surfacerendercontext_setup_constants(context);
}

static inline x_fp16x16 calculate_u_div_z(const X_AE_SurfaceRenderContext* context, int x, int y)
{
    return x * context->sDivZ.uOrientationStep + y * context->sDivZ.vOrientationStep + context->sDivZ.origin;
}

static inline x_fp16x16 calculate_v_div_z(const X_AE_SurfaceRenderContext* context, int x, int y)
{
    return x * context->tDivZ.uOrientationStep + y * context->tDivZ.vOrientationStep + context->tDivZ.origin;
}

static inline void clamp_texture_coord(const X_AE_SurfaceRenderContext* context, x_fp16x16* u, x_fp16x16* v)
{
    if(*u < 0) *u = 16;
    else if(*u >= context->surfaceW) *u = context->surfaceW - X_FP16x16_ONE;
    
    if(*v < 0)*v = 16;
    else if(*v >= context->surfaceH) *v = context->surfaceH - X_FP16x16_ONE;
}

static int calculate_z_at_screen_point(const X_AE_SurfaceRenderContext* context, int x, int y)
{
    x_fp2x30 invZ = (x_ae_surface_calculate_inverse_z_at_screen_point(context->surface, x, y));
    
    return x_fastrecip(invZ >> 10);
}

static inline void calculate_u_and_v_at_screen_point(const X_AE_SurfaceRenderContext* context, int x, int y, x_fp16x16* u, x_fp16x16* v)
{
    x_fp16x16 uDivZ = calculate_u_div_z(context, x, y);
    x_fp16x16 vDivZ = calculate_v_div_z(context, x, y);
    
    int z = calculate_z_at_screen_point(context, x, y);
    
    *u = uDivZ * z + context->sDivZ.adjust;
    *v = vDivZ * z + context->tDivZ.adjust;
    
    clamp_texture_coord(context, u, v);
}

static inline X_Color get_texel(const X_AE_SurfaceRenderContext* context, x_fp16x16 u, x_fp16x16 v)
{
    int uu = (u >> 16);
    int vv = (v >> 16);
    
#if 1
    uu = uu % context->surfaceTexture.getW();
    vv = vv % context->surfaceTexture.getH();
#endif
    
    return context->surfaceTexture.getTexels()[vv * context->surfaceTexture.getW() + uu];
}

static inline void __attribute__((hot)) fill_solid_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span, X_Color color)
{
    X_Texture* screenTex = context->renderContext->canvas;
    X_Color* scanline = screenTex->getRow(span->y);

    for(int x = span->x1; x < span->x2; ++x)
    {
        // temp check
        if(scanline[x] == 0)
        scanline[x] = color;
    }
}

static inline void __attribute__((hot)) x_ae_surfacerendercontext_render_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span)
{
    int y = span->y;
    
    //span->y *= 2;
    
    X_Texture* screenTex = context->renderContext->canvas;
    X_Color* scanline = screenTex->getRow(span->y);
    //x_fp0x16* zbuf = context->renderContext->zbuf + span->y * screenTex->getW();
    
    x_fp16x16 invZ = x_ae_surface_calculate_inverse_z_at_screen_point(context->surface, span->x1, y) >> 10;
    x_fp16x16 dInvZ = context->surface->zInverseXStep >> 10;
    
    x_fp16x16 u, v;
    calculate_u_and_v_at_screen_point(context, span->x1, y, &u, &v);
    
    int x = span->x1;
    while(x < span->x2 - 16)
    {
        x_fp16x16 nextU, nextV;
        calculate_u_and_v_at_screen_point(context, x + 16, y, &nextU, &nextV);
        
        x_fp16x16 dU = (nextU - u) >> 4;
        x_fp16x16 dV = (nextV - v) >> 4;
        
        for(int i = 0; i < 16; ++i)
        {
            X_Color texel = get_texel(context, u, v);
            
            if(scanline[x] == 0)

            scanline[x] = texel;
//             scanline[x * 2] = texel;
//             scanline[x * 2 + 1] = texel;
//             scanline[x * 2 + screenTex->w] = texel;
//             scanline[x * 2 + screenTex->w + 1] = texel;
            
            //zbuf[x] = invZ;
            
            //invZ += dInvZ;
            u += dU;
            v += dV;
            
            ++x;
        }
    }
    
    if(x == span->x2)
        return;
    
    x_fp16x16 nextU, nextV;
    calculate_u_and_v_at_screen_point(context, span->x2, y, &nextU, &nextV);
    
    int dX = span->x2 - x;
    x_fp16x16 dU = (nextU - u) / dX;
    x_fp16x16 dV = (nextV - v) / dX;
    
    while(x < span->x2)
    {
        //scanline[x] = get_texel(context, u, v);
        //zbuf[x] = invZ;
        
        X_Color texel = get_texel(context, u, v);
   
        if(scanline[x] == 0)

        scanline[x] = texel;
//         scanline[x * 2] = texel;
//         scanline[x * 2 + 1] = texel;
//         scanline[x * 2 + screenTex->w] = texel;
//         scanline[x * 2 + screenTex->w + 1] = texel;
//         
        invZ += dInvZ;
        u += dU;
        v += dV;
        
        ++x;
    }
}

extern "C"
{

void draw_surface_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span);
void draw_surface(X_AE_SurfaceRenderContext* context, X_AE_Span* span);

}

static void merge_adjacent_spans(X_AE_Span* head)
{
    X_AE_Span* prev = head;
    X_AE_Span* span = head->next;
    
    while(span)
    {
        // Faster way to check: (prev->y == y && prev->x2 == left)
        bool extendSpan = ((prev->y ^ span->y) | (prev->x2 ^ span->x1)) == 0;
        
        if(extendSpan)
        {
            prev->x2 = span->x2;
            prev->next = span->next;
        }
        else
            prev = span;
        
        span = span->next;
    }
}

void __attribute__((hot)) x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context)
{
    if(((context->renderContext->renderer->renderMode) & 1) == 0)
        return;
    
    context->surface->last->next = NULL;
    
    if(context->surface->inSubmodel)
        merge_adjacent_spans(context->surface->spanHead.next);
    
 #ifdef __nspire__
    draw_surface(context, context->surface->spanHead.next);
    return;
 #endif
     
    if(context->surface->flags.hasFlag(SURFACE_FILL_SOLID) || true)
    {
        X_Color fillColor = context->surface->bspSurface->color; // context->surface->getSolidFillColor();

        for(X_AE_Span* span = context->surface->spanHead.next; span != NULL; span = span->next)
        {
            fill_solid_span(context, span, fillColor);
        }
    }
    else
    {
        for(X_AE_Span* span = context->surface->spanHead.next; span != NULL; span = span->next)
        {
            x_ae_surfacerendercontext_render_span(context, span);
        }
    }
}

