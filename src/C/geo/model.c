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
#include "memory/X3D_alloc.h"

void x3d_model_create_dynamically_allocated_model(X3D_Model* model) {
  model->total_prisms = 0;
  model->prisms = NULL;
  
  model->total_v = 0;
  model->v = NULL;
  
  model->total_v_runs = 0;
  model->v_runs = NULL;
}

uint16 x3d_model_add_vertex(X3D_Model* model, X3D_Vex3D* v) {
  uint16 i;
  for(i = 0; i < model->total_v; ++i) {
    if(x3d_vex3d_equal(model->v + i, v)) {
      return i;
    }
  }
  
  model->v = realloc(model->v, sizeof(X3D_Vex3D) * (model->total_v + 1));
  model->v[model->total_v] = *v;
  
  return model->total_v++;
}

uint16 x3d_model_add_vertex_run(X3D_Model* model, X3D_Vex3D* v, uint16 total_v) {
  uint16 run_start = model->total_v_runs;
  
  model->total_v_runs += total_v;
  model->v_runs = realloc(model->v_runs, sizeof(uint16) * model->total_v_runs);
  
  uint16 i;
  for(i = 0; i < total_v; ++i) {
    model->v_runs[i + run_start] = x3d_model_add_vertex(model, v + i);
  }
  
  return run_start;
}

void x3d_model_add_prism3d(X3D_Model* model, X3D_Prism3D* prism, X3D_Vex3D position_relative_to_center) {
  model->prisms = realloc(model->prisms, (model->total_prisms + 1) * sizeof(X3D_ModelPrism));
  
  X3D_Prism3D new_prism = { .v = alloca(1000) };
  
  x3d_prism3d_copy(&new_prism, prism);
  
  // Move the prism to its relative center, then move it relative to the center of the model
  X3D_Vex3D move_to_prism_center;
  x3d_prism3d_center(&new_prism, &move_to_prism_center);
  x3d_vex3d_neg(&move_to_prism_center);
  
  X3D_Vex3D translation = x3d_vex3d_add(&move_to_prism_center, &position_relative_to_center);
  x3d_prism3d_translate(&new_prism, &translation);
  
  X3D_ModelPrism* model_prism = model->prisms + model->total_prisms;
  
  model_prism->v_run_start = x3d_model_add_vertex_run(model, new_prism.v, new_prism.base_v * 2);
  model_prism->base_v = new_prism.base_v;
  
  ++model->total_prisms;
}

void x3d_model_clone_using_slab_allocator(X3D_Model* src, X3D_Model* dest) {
  /*
  dest->total_prisms = src->total_prisms;
  dest->prisms = x3d_slab_alloc(src->total_prisms * sizeof(X3D_Prism3D));
  
  uint16 i;
  for(i = 0; i < src->total_prisms; ++i) {
    dest->prisms[i].v = x3d_slab_alloc(x3d_prism3d_size(src->prisms[i].base_v));
    x3d_prism3d_copy(dest->prisms + i, src->prisms + i);
  }
  */
}

void x3d_model_get_geo(X3D_Model* model, uint16 prism_id, X3D_Prism3D* dest) {
  X3D_ModelPrism* prism = model->prisms + prism_id;
  
  uint16 i;
  for(i = 0; i < prism->base_v * 2; ++i) {
    dest->v[i] = model->v[model->v_runs[prism->v_run_start + i]];
  }
  
  dest->base_v = prism->base_v;
}

