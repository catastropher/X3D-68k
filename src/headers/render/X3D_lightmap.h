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

#define X3D_LIGHTMAP_SCALE 8

enum {
    X3D_PLANE_XY,
    X3D_PLANE_XZ,
    X3D_PLANE_YZ
};

typedef struct X3D_PlanarProjection {
    int16 plane_type;
    X3D_Polygon2D poly;
    int16 min_x, min_y;
    int16 max_x, max_y;
    X3D_Plane poly_plane;
} X3D_PlanarProjection;

typedef struct X3D_LightMap {
    int16 w, h;
    uint8* data;
} X3D_LightMap;

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
    
    X3D_LightMap* maps;
    uint16 total_maps;
    
    X3D_Level* level;
    X3D_PlanarProjection* proj;
    
    _Bool** set;
} X3D_LightMapContext;

uint8 x3d_lightmap_get_value(X3D_LightMap* map, uint16 x, uint16 y);
void x3d_planarprojection_project_point(X3D_PlanarProjection* proj, X3D_Vex3D* v, X3D_Vex2D* dest);
void x3d_lightmapcontext_init(X3D_LightMapContext* context, X3D_Level* level);
void x3d_lightmap_bilinear_filter(X3D_LightMap* map);

