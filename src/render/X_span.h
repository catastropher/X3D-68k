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

#pragma once

#include "math/X_fix.h"
#include "render/X_Texture.h"

struct X_AE_Surface;
struct X_Viewport;
struct X_RenderContext;
struct X_BspFaceTexture;

typedef struct X_AE_Span
{
    short x1;
    short x2;
    short y;
} X_AE_Span;

typedef struct X_AE_TextureVar
{
    x_fp16x16 uOrientationStep;
    x_fp16x16 vOrientationStep;
    x_fp16x16 origin;
    x_fp16x16 adjust;
} X_AE_TextureVar;

typedef struct X_AE_SurfaceRenderContext
{
    struct X_AE_Surface* surface;
    struct X_Viewport* viewport;
    struct X_RenderContext* renderContext;
    struct X_BspFaceTexture* faceTexture;
    int mipLevel;
    
    X_AE_TextureVar sDivZ;
    X_AE_TextureVar tDivZ;
    
    X_Texture faceTex;
    int uMask;
    int vMask;
} X_AE_SurfaceRenderContext;

void x_ae_surfacerendercontext_init(X_AE_SurfaceRenderContext* context, struct X_AE_Surface* surface, struct X_RenderContext* renderContext, int mipLevel);
void x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context);

