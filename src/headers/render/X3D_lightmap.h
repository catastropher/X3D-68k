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

#include "X3D_common.h"
#include "level/X3D_level.h"
#include "X3D_polygon.h"
#include "X3D_texture.h"

typedef struct X3D_SpotLight {
    X3D_Vex3D pos;
    X3D_Mat3x3 mat;
    fp0x16 intensity;
    float light_angle;
    X3D_Vex3D_angle256 orientation;
} X3D_SpotLight;

typedef struct X3D_LightMapContext {
    X3D_SpotLight* lights;
    uint16 total_light;
    
    X3D_Texture* lightmaps;
    uint16 total_lightmaps;
    
    X3D_Level* level;
    X3D_PlanarProjection* proj;
    
    X3D_Texture* surfaces;
} X3D_LightMapContext;

void x3d_lightmapcontext_init(X3D_LightMapContext* context, X3D_Level* level);
void x3d_lightmap_bilinear_filter(X3D_Texture* map);
void x3d_lightmap_build(X3D_SpotLight* light, X3D_LightMapContext* context);
void x3d_lightmapcontext_build_surfaces(X3D_LightMapContext* context, X3D_Texture* level_tex);


static inline X3D_Texture* x3d_lightmapcontext_get_surface_for_level_face(X3D_LightMapContext* context, X3D_SegFaceID face) {
    return context->surfaces + face;
}
