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

static void x_ae_surfacerendercontext_init_sdivz(X_AE_SurfaceRenderContext* context)
{
    X_Vec3 sAxis;
    X_BspFaceTexture* tex = context->faceTexture;
    x_mat4x4_rotate_normal(context->renderContext->viewMatrix, &tex->uOrientation, &sAxis);
    
    X_AE_TextureVar* sDivZ = &context->sDivZ;
    sDivZ->uOrientationStep = (sAxis.x / context->viewport->distToNearPlane) >> context->mipLevel;
    sDivZ->vOrientationStep = (sAxis.y / context->viewport->distToNearPlane) >> context->mipLevel;
    
    //x_vec3_fp16x16_print(&tex->vOrientation, "V");
    
    // TODO: move these into viewport struct
    int centerX = context->viewport->screenPos.x + context->viewport->w / 2;
    int centerY = context->viewport->screenPos.y + context->viewport->h / 2;
    
    sDivZ->origin = (sAxis.z >> context->mipLevel) - centerX * sDivZ->uOrientationStep - centerY * sDivZ->vOrientationStep;
}

static void x_ae_surfacerendercontext_init_tdivz(X_AE_SurfaceRenderContext* context)
{
    X_Vec3 tAxis;
    X_BspFaceTexture* tex = context->faceTexture;
    x_mat4x4_rotate_normal(context->renderContext->viewMatrix, &tex->vOrientation, &tAxis);
    
    X_AE_TextureVar* tDivZ = &context->tDivZ;
    tDivZ->uOrientationStep = (tAxis.x / context->viewport->distToNearPlane) >> context->mipLevel;
    tDivZ->vOrientationStep = (tAxis.y / context->viewport->distToNearPlane) >> context->mipLevel;
    
    // TODO: move these into viewport struct
    int centerX = context->viewport->screenPos.x + context->viewport->w / 2;
    int centerY = context->viewport->screenPos.y + context->viewport->h / 2;
    
    tDivZ->origin = (tAxis.z >> context->mipLevel) - centerX * tDivZ->uOrientationStep - centerY * tDivZ->vOrientationStep;
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
}

static x_fp16x16 calculate_u_div_z(X_AE_SurfaceRenderContext* context, int x, int y)
{
    return x * context->sDivZ.uOrientationStep + y * context->sDivZ.vOrientationStep + context->sDivZ.origin;
}

static x_fp16x16 calculate_v_div_z(X_AE_SurfaceRenderContext* context, int x, int y)
{
    return x * context->tDivZ.uOrientationStep + y * context->tDivZ.vOrientationStep + context->tDivZ.origin;
}

float g_zbuf[480][640];

static inline void x_ae_surfacerendercontext_render_span(X_AE_SurfaceRenderContext* context, X_AE_Span* span)
{
    X_Texture* screenTex = &context->renderContext->screen->canvas.tex;
    X_Color* scanline = screenTex->texels + span->y * screenTex->w;
    
    X_Texture faceTex;
    X_BspLevel* level = context->renderContext->level;
    x_bsplevel_get_texture(level, context->faceTexture->texture - level->textures, 0, &faceTex);
    
    for(int i = span->x1; i < span->x2; ++i)
    {
        x_fp16x16 uDivZ = calculate_u_div_z(context, i, span->y);
        x_fp16x16 vDivZ = calculate_v_div_z(context, i, span->y);
        x_fp0x30 invZ = x_ae_surface_calculate_inverse_z_at_screen_point(context->surface, i, span->y);
        
        //x_fp16x16 u = ((long long)uDivZ * invZ) >> 16;
        //x_fp16x16 v = ((long long)vDivZ * invZ) >> 16;
        
        int u = (uDivZ / 65536.0) / ((float)invZ / (1 << 30));
        int v = (vDivZ / 65536.0) / ((float)invZ / (1 << 30));
        
//         if(i == 0)
//             printf("U: %d V: %d\n", u, v);
        
        if(u >= 0)
            u = (u % faceTex.w);
        else
        {
            while(u < 0)
                u += faceTex.w;
        }
        
        
        if(v >= 0)
            v = (v % faceTex.h);
        else
        {
             while(v < 0)
                 v += faceTex.h;
        }
        
        scanline[i] = x_texture_get_texel(&faceTex, u, v);
    }
}

void x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context)
{
    for(int i = 0; i < context->surface->totalSpans; ++i)
    {
        x_ae_surfacerendercontext_render_span(context, context->surface->spans + i);
    }
}



