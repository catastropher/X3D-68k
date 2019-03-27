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

#include "geo/Vec3.hpp"


struct EngineContext;
struct OldRenderer;
struct Camera;
struct X_Frustum;
struct Texture;
struct Mat4x4;
struct BspLevel;
struct BspModel;
struct Screen;

typedef struct X_RenderContext
{
    OldRenderer* renderer;
    Camera* cam;
    Screen* screen;
    Texture* canvas;
    x_fp0x16* zbuf;
    X_Frustum* viewFrustum;
    Mat4x4* viewMatrix;
    EngineContext* engineContext;
    BspLevel* level;
    
    int currentFrame;
    Vec3fp camPos;
} X_RenderContext;

