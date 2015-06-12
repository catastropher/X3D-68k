// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_fix.h"

#ifndef NDEBUG

typedef struct {
  uint16 type;
  void* addr;
  const char* name;
} X3D_Param;

typedef struct {
  const char* name;
  uint16 total_p;
  X3D_Param p[8];
} X3D_FunctionCall;

X3D_FunctionCall x3d_call_stack[32];
short x3d_call_stack_top = -1;

void* x3d_functioncall_enter(const char* name) {
  x3d_call_stack[++x3d_call_stack_top].name = name;
  x3d_call_stack[x3d_call_stack_top].total_p = 0;

  return &x3d_call_stack[x3d_call_stack_top];
}

void x3d_functioncall_return(void* ptr) {
  --x3d_call_stack_top;
}

void x3d_functioncall_param_add(void* callentry, const char* name, int type, void* param_ptr) {
  X3D_FunctionCall* f = callentry;

  f->p[f->total_p].name = name;
  f->p[f->total_p].type = type;
  f->p[f->total_p].addr = param_ptr;
  f->total_p++;
}

void x3d_print_stacktrace() {
  int16 i, d;

  printf("========Stack Trace========\n");
  printf("Entries: %d\n\n", x3d_call_stack_top + 1);

  for(i = x3d_call_stack_top; i >= 0; --i) {
    printf("-%s\n", x3d_call_stack[i].name);

    for(d = 0; d < x3d_call_stack[i].total_p; d++) {
      printf("    %s: ", x3d_call_stack[i].p[d].name);

      switch(x3d_call_stack[i].p[d].type) {
      case PARAM_INT16:
        printf("%d", *((int16*)x3d_call_stack[i].p[d].addr));
        break;
      case PARAM_INT32:
        printf("%ld", *((int32*)x3d_call_stack[i].p[d].addr));
        break;
      default:
        printf("<unknown type>");
        break;
      }

      printf("\n");
    }
  }

  printf("===========================\n");
}

// Throws an error, prints out the message, and then quits the program
void x3d_error(const char* format, ...) {
  char buf[512];
  va_list list;

  va_start(list, format);
  vsprintf(buf, format, list);

#ifdef __TIGCC__
  PortRestore();
  clrscr();

#endif
  printf("Error: %s\nPress Esc to quit\n\n", buf);

  x3d_print_stacktrace();

#ifdef __TIGCC__
  while(!_keytest(RR_ESC));
#endif

  exit(-1);
}

#endif



void test_b(int16 x) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_INT16, x);

  add_int16_overflow(-32767, -50);
}

void test_a(int16 x) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_INT16, x);

  test_b(x);
}

