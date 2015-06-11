#pragma once

#include "X3D_fix.h"

#ifdef _WIN32
#define __attribute__(...) 
#endif

#ifdef NDEBUG
#define x3d_error(...) ;
#define x3d_assert(...) ;
#define x3d_errorif(...) ;

#else

void x3d_error(const char* format, ...) __attribute__((noreturn));

#define x3d_errorif(_cond, _format, ...) {if(_cond) x3d_error(_format, ##__VA_ARGS__);}
#define x3d_assert(_cond) x3d_errorif(!(_cond), "Assertion failed!\nFile: %s\nLine: %d\nFunction: %s\nCond: %s\n",__FILE__,__LINE__,__FUNCTION__,#_cond)

void x3d_sprintf(char* buf, const char* format, ...);

#endif

#define X3D_STACK_TRACE void* x3d_functioncall_entry __attribute__ ((__cleanup__(x3d_functioncall_return))) = x3d_functioncall_enter(__FUNCTION__)
#define X3D_PARAM(_type, _param) x3d_functioncall_param_add(x3d_functioncall_entry, &_param);

void* x3d_functioncall_enter(const char* name);
void x3d_functioncall_return(void* ptr);
void x3d_functioncall_param_add(void* callentry, int type, void* param_ptr);

