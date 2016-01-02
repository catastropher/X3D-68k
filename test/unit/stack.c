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

#include "X3D.h"

void test_stack() {
  // Make sure odd-sized allocs return an even
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    
    void* ptr = x3d_stack_alloc(&stack, 5);
    
    x3d_assert(((uint64)ptr) & 2);
  }
  
  // Resetting the stack
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    x3d_stack_alloc(&stack, 5);
    
    void* stack_ptr = x3d_stack_save(&stack);
    
    x3d_stack_alloc(&stack, 5);
    x3d_stack_alloc(&stack, 5);
    x3d_stack_alloc(&stack, 5);
    x3d_stack_alloc(&stack, 5);
    x3d_stack_alloc(&stack, 5);
    x3d_stack_restore(&stack, stack_ptr);
    
    x3d_assert(x3d_stack_save(&stack) == stack_ptr);
  }
  
  // These should crash
  
  // Stack overflow
#if 0
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    x3d_stack_alloc(&stack, 1025);
  }
#endif

  // Stack overflow
#if 0
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    x3d_stack_alloc(&stack, 513);
    x3d_stack_alloc(&stack, 511);
  }
#endif

  // Invalid stack pointer (too high)
#if 0
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    x3d_stack_restore(&stack, mem + 1025);
  }
#endif

  // Invalid stack pointer (too low)
#if 0
  {
    X3D_Stack stack;
    char mem[1024];
    
    x3d_stack_init(&stack, mem, 1024);
    x3d_stack_restore(&stack, mem - 1);
  }
#endif

}