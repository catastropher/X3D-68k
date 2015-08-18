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
#include "X3D_error.h"

#ifndef NDEBUG

/// A function call parameter used for debugging
typedef struct {
  X3D_ParamType type;           ///< Type of the parameter
  void* addr;                   ///< Address of where the valus is stored
  const char* name;             ///< Name of the parameter
} X3D_Param;

/// A function call type used for stack tracing
typedef struct {
  const char* name;             ///< Name of the function being called
  uint16 total_p;               ///< Total number of parameters
  X3D_Param p[8];               ///< Information for each parameter
} X3D_FunctionCall;

/// Stack of function calls for stack tracing
static X3D_FunctionCall x3d_call_stack[32];

/// Top index for the stack trace
static int16 x3d_call_stack_top = -1;

/**
* Adds a function call entry to the stack trace.
*
* @param name - name of the function
*
* @return A pointer to the call entry.
* @note For internal use only. See @ref X3D_STACK_TRACE to enable stack tracing
*     for a particular function. This function is called from the @ref X3D_STACK_TRACE
*     macro.
*/
void* x3d_functioncall_enter(const char* name) {
  x3d_call_stack[++x3d_call_stack_top].name = name;
  x3d_call_stack[x3d_call_stack_top].total_p = 0;

  return &x3d_call_stack[x3d_call_stack_top];
}

/**
* Removes a function call from the stack trace.
*
* @param ptr - a dummy pointer variable
*
* @return nothing
* @note For internal use only. This is called automatically as a destructor for
*     stack tracing.
*/
void x3d_functioncall_return(void* ptr) {
  --x3d_call_stack_top;
}

/**
* Adds a parameter to a stack trace entry.
*
* @param callentry  - pointer to the stack trace function call entry
* @param name       - name of the parameter
* @param type       - type of the parameter
* @param param_ptr  - pointer to the paramater value
*
* @return nothing
* @note For internal use only. See @ref X3D_PARAM to add a paramter to a function call
*     stack trace entry.
*/
void x3d_functioncall_param_add(void* callentry, const char* name, int type, void* param_ptr) {
  X3D_FunctionCall* f = callentry;

  f->p[f->total_p].name = name;
  f->p[f->total_p].type = type;
  f->p[f->total_p].addr = param_ptr;
  f->total_p++;
}

/**
* Prints out the stack trace.
*
* @return nothing
* @todo Implement more parameter types that can be printed 
*/
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

/**
* Throws an error, prints out the error message, and aborts.
*
* @param format - format string in printf format
*
* @return nothing
* @todo Allow errors to be caught in try/catch block?
*/
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
