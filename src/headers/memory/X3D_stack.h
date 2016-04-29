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

typedef struct X3D_Stack {
  void* base;
  void* ptr;
  uint32 size;
} X3D_Stack;

#define X3D_STACK_ALLOC_TYPE(_stack, _type, _total) (_type* )x3d_stack_alloc(_stack, sizeof(_type) * (_total))

void x3d_stack_init(X3D_Stack* stack, void* mem, uint32 mem_size);
void* x3d_stack_alloc(X3D_Stack* stack, uint16 size);
void* x3d_stack_save(X3D_Stack* stack);
void x3d_stack_restore(X3D_Stack* stack, void* ptr);
void x3d_stack_reset(X3D_Stack* stack);

