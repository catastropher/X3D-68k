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

#include "geo/X3D_model.h"
#include "X3D_screen.h"
#include "render/geo/X3D_render_prism.h"
#include "X3D_camera.h"
#include "render/geo/X3D_render_line.h"

void x3d_model_render_wireframe_triangle(X3D_Model* model, X3D_CameraObject* cam, int16* v, X3D_ColorIndex color) {
    for(int i = 0; i < 3; ++i) {
        int next = (i + 1) % 3;
        
        X3D_Ray3D ray = x3d_ray3d_make(model->v[v[i]], model->v[v[next]]);
        x3d_ray3d_render(&ray, cam, color);
    }
}

void x3d_model_render_wireframe(X3D_Model* model, X3D_CameraObject* cam, X3D_ColorIndex color) {
    for(int16 i = 0; i < model->total_triangles; ++i) {
        int16 v[3] = {
            model->triangle_vertices[i * 3 + 0],
            model->triangle_vertices[i * 3 + 1],
            model->triangle_vertices[i * 3 + 2]
        };
        
        x3d_model_render_wireframe_triangle(model, cam, v, color);
    }
}
