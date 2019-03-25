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

#include "render/Viewport.hpp"
#include "physics/BoxCollider.hpp"
#include "memory/BitSet.hpp"
#include "level/PotentiallyVisibleSet.hpp"

struct EngineContext;
struct X_RenderContext;
struct Screen;
struct BspLeaf;

enum CameraobjectFlags
{
    CAMERA_OVERRIDE_PVS = 1
};

class EntityBuilder;

struct Camera
{
    void updateFrustum();
    void overrideBspLeaf(int leafId, BspLevel* level);

    Flags<CameraobjectFlags> flags;
    Vec3fp position;        // TODO: this should be removed
    
    Viewport viewport;
    Mat4x4 viewMatrix;
    struct BspLeaf* currentLeaf;
    struct BspLeaf* lastLeaf;
    DecompressedLeafVisibleSet pvsForCurrentLeaf;
    
    void (*screenResizeCallback)(struct Camera* cam, struct Screen* screen, fp fov);
};

void x_cameraobject_render(Camera* cam, struct X_RenderContext* renderContext);

static inline Vec3fp x_cameraobject_get_position(Camera* cam)
{
    return cam->position;
}


