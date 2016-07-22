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

#include "X3D_common.h"
#include "X3D_object.h"
#include "X3D_enginestate.h"
#include "X3D_assert.h"
#include "memory/X3D_handle.h"
#include "memory/X3D_alloc.h"
#include "X3D_render.h"
#include "X3D_wallportal.h"

void x3d_objectmanager_init() {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  objectman->total_object_types = 0;
  objectman->types = NULL;
}

X3D_ObjectType* x3d_objecttype_create(uint16 obj_type_id) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  if(obj_type_id >= objectman->total_object_types) {
    objectman->total_object_types = obj_type_id + 1;
    objectman->types = realloc(objectman->types, sizeof(X3D_ObjectType) * objectman->total_object_types);
  }
  
  X3D_ObjectType* type = objectman->types + obj_type_id;
  
  type->event_handler = NULL;
  type->name[0] = '\0';
  
  return type;
}

X3D_ObjectType* x3d_objecttype_get_ptr(uint16 obj_type_id) {
  return x3d_objectmanager_get()->types + obj_type_id;
}

