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

#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_screen.h"
#include "render/geo/X3D_render_line.h"

void x3d_prism3d_render_wireframe(X3D_Prism3D* prism, X3D_CameraObject* cam, X3D_Color color) {
  uint16 i;
  for(i = 0; i < prism->base_v * 3; ++i) {
    X3D_Ray3D edge;
    x3d_prism3d_get_edge(prism, i, &edge);
    x3d_ray3d_render(&edge, cam, color);
  }
}

void x3d_prism3d_render_wireframe_with_colored_edges(X3D_Prism3D* prism, X3D_CameraObject* cam, X3D_ColorIndex* color_for_each_edge) {
    for(uint16 i = 0; i < prism->base_v * 3; ++i) {
        X3D_Ray3D edge;
        x3d_prism3d_get_edge(prism, i, &edge);
        x3d_ray3d_render(&edge, cam, color_for_each_edge[i]);
    }
}

