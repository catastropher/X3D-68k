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

#include "math/FixedPoint.hpp"
#include "render/Texture.hpp"

struct X_AE_Surface;
struct Viewport;
struct X_RenderContext;
struct BspFaceTexture;

typedef struct X_AE_Span
{
    int x1;
    int x2;
    int y;
    struct X_AE_Span* next;
} X_AE_Span;

typedef struct X_AE_TextureVar
{
    fp uOrientationStep;
    fp vOrientationStep;
    fp origin;
    fp adjust;
} X_AE_TextureVar;

typedef struct X_AE_SurfaceRenderContext
{
    // !!! DO NOT REORDER THESE, THE ASM CODE EXPECTS THEM IN THIS ORDER !!!
    // The numbers in the comments are the offset (in bytes) from the beginning
    // of the struct
    int invZStepX;          // 0
    int invZStepY;          // 4
    int invZOrigin;         // 8
    
    int uStepX;             // 12
    int uStepY;             // 16
    int uOrigin;            // 20
    
    int vStepX;             // 24
    int vStepY;             // 28
    int vStepOrigin;        // 32
    
    int invZStepXNeg;       // 36 (0)
    int uStepXNeg;          // 40 (4)
    int vStepXNeg;          // 44 (8)
    
    X_Color* screen;        // 48 (12)
    
    int texW;               // 52 (16)
    
    int uAdjust;            // 56 (20)
    int vAdjust;            // 60 (24)
    
    fp surfaceW;     // 64 (28)
    fp surfaceH;     // 68 (32)
    
    X_Color* surfaceTexels; // 72 (36)
    
    const fp* recipTab;    // 76 (40)
    
    
    X_AE_TextureVar sDivZ;
    X_AE_TextureVar tDivZ;
    
    struct X_AE_Surface* surface;
    struct Viewport* viewport;
    struct X_RenderContext* renderContext;
    struct BspFaceTexture* faceTexture;
    int mipLevel;
    
    X_Texture surfaceTexture;
} X_AE_SurfaceRenderContext;

void x_ae_surfacerendercontext_init(X_AE_SurfaceRenderContext* context, struct X_AE_Surface* surface, struct X_RenderContext* renderContext, int mipLevel);
void x_ae_surfacerendercontext_render_spans(X_AE_SurfaceRenderContext* context);

