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

void x3d_model_create_dynamically_allocated_model(X3D_Model* model) {
  model->total_prisms = 0;
  model->prisms = NULL;
}

void x3d_model_add_prism3d(X3D_Model* model, X3D_Prism3D* prism, X3D_Vex3D position_relative_to_center) {
  model->prisms = realloc(model->prisms, (model->total_prisms + 1) * sizeof(X3D_Prism3D));
  
  X3D_Prism3D* new_prism = model->prisms + model->total_prisms;
  new_prism->v = malloc(x3d_prism3d_size(prism->base_v));
  
  x3d_prism3d_copy(new_prism, prism);
  
  // Move the prism to its relative center, then move it relative to the center of the model
  X3D_Vex3D move_to_prism_center;
  x3d_prism3d_center(new_prism, &move_to_prism_center);
  x3d_vex3d_neg(&move_to_prism_center);
  
  X3D_Vex3D translation = x3d_vex3d_add(&move_to_prism_center, &position_relative_to_center);
  x3d_prism3d_translate(new_prism, &translation);
  
  ++model->total_prisms;
}
