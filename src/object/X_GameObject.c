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

#include "X_GameObject.h"
#include "engine/X_EngineContext.h"

X_GameObject* x_gameobject_new(X_EngineContext* context, size_t objectSize)
{
    int objectHandle;
    X_GameObject* newObject = x_factory_alloc(&context->gameObjectFactory, objectSize, &objectHandle);
    newObject->id = objectHandle;
    
    return newObject;
}

void x_gameobject_extract_view_vectors(const X_GameObject* obj, X_Vec3* forwardDest, X_Vec3* rightDest, X_Vec3* upDest)
{
    X_Mat4x4 mat;
    x_quaternion_to_mat4x4(&obj->orientation, &mat);
    
    X_Vec4 right;
    x_mat4x4_get_column(&mat, 0, &right);
    *rightDest = x_vec4_to_vec3(&right);
    
    X_Vec4 up;
    x_mat4x4_get_column(&mat, 1, &up);
    *upDest = x_vec4_to_vec3(&up);
    *upDest = x_vec3_neg(upDest);
    
    X_Vec4 forward;
    x_mat4x4_get_column(&mat, 2, &forward);
    *forwardDest = x_vec4_to_vec3(&forward);
}

