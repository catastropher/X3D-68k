#pragma once

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

