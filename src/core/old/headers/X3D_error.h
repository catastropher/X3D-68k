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

#pragma once

void x3d_error(const char* format, ...);

// Keeps Visual Studio from getting upset with gcc attributes
#ifdef _WIN32
#define __attribute__(...) 
#endif

///< Shortcut for if(cond) x3d_error()
#define x3d_errorif(_cond, _format, ...) {if(_cond) x3d_error(_format, ##__VA_ARGS__);}

// If debug mode is disabled, define these macros to be empty
#ifdef NDEBUG
//#define x3d_error(...) ;
#define x3d_assert(...) ;
//#define x3d_errorif(...) ;

#define X3D_STACK_TRACE ;
#define X3D_PARAM(...) ;

#define X3D_LOG_WAIT(_context, _format, ...) {printf(_format, ##__VA_ARGS__); LCD_restore((_context)->screen); while(!_keytest(RR_APPS)); while(_keytest(RR_APPS));}

#else

//=============================================================================
// Macros
//=============================================================================

///< Replacement for assert()
#define x3d_assert(_cond) x3d_errorif(!(_cond), "Assertion failed!\nFile: %s\nLine: %d\nFunction: %s\nCond: %s\n",__FILE__,__LINE__,__FUNCTION__,#_cond)

///< Put at the top of a function to enable stack tracing for that function
#define X3D_STACK_TRACE void* x3d_functioncall_entry __attribute__ ((__cleanup__(x3d_functioncall_return))) = x3d_functioncall_enter(__FUNCTION__)

///< Put under @ref X3D_STACK_TRACE to add a paramater to the stack trace 
#define X3D_PARAM(_type, _param) x3d_functioncall_param_add(x3d_functioncall_entry,#_param,_type, &_param);

/// @todo document
#define X3D_LOG_WAIT(_context, _format, ...) {printf(_format, ##__VA_ARGS__); LCD_restore(_context->screen); while(!_keytest(RR_APPS)); while(_keytest(RR_APPS));}

//=============================================================================
// Types
//=============================================================================

/// Parameter types that can be added to a stack trace.
typedef enum {
  PARAM_INT8,
  PARAM_UINT8,
  PARAM_INT16,
  PARAM_UINT16,
  PARAM_INT32,
  PARAM_UINT32,
  PARAM_PTR,
  PARAM_VEX3D_INT16
} X3D_ParamType;

//=============================================================================
// Function declarations
//=============================================================================
void x3d_error(const char* format, ...) __attribute__((noreturn));
void* x3d_functioncall_enter(const char* name);
void x3d_functioncall_return(void* ptr);
void x3d_functioncall_param_add(void* callentry, const char* name, int type, void* param_ptr);

#endif

