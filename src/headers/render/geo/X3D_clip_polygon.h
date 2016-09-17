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
#include "render/geo/X3D_render_polygon.h"
#include "X3D_plane.h"

_Bool x3d_rasterpolygon3d_clip_to_frustum(X3D_RasterPolygon3D* src, X3D_Frustum* f, X3D_RasterPolygon3D* dest);
X3D_Frustum* x3d_get_view_frustum(X3D_CameraObject* cam);
