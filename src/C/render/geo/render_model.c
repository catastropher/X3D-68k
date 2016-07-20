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

void x3d_model_render(X3D_Model* model, X3D_CameraObject* cam, X3D_Color color, X3D_Vex3D_angle256 rot_angle, X3D_Vex3D pos) {
  X3D_Prism3D temp_prism = { .v = alloca(1000) };
  X3D_Mat3x3 rot_mat;
  x3d_mat3x3_construct(&rot_mat, &rot_angle);
  
  uint16 i;
  for(i = 0; i < model->total_prisms; ++i) {
    x3d_prism3d_copy(&temp_prism, model->prisms + i);
    
    uint16 d;
    for(d = 0; d < temp_prism.base_v * 2; ++d) {
      X3D_Vex3D temp_v;
      x3d_vex3d_int16_rotate(&temp_v, temp_prism.v + d, &rot_mat);
      temp_prism.v[d] = temp_v;
    }
    
    x3d_prism3d_translate(&temp_prism, &pos);
    x3d_prism3d_render_wireframe(&temp_prism, cam, color);
  }
}

