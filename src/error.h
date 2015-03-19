// Header File
// Created 3/9/2015; 11:22:22 AM

void error(const char* format, ...) __attribute__((noreturn));

#define errorif(_cond, _format, ...) {if(_cond) error(_format, ##__VA_ARGS__);}

#define xassert(_cond) errorif(!(_cond), "Assertion failed!\nFile: %s\nLine: %d\nFunction: %s\nCond: %s\n",__FILE__,__LINE__,__FUNCTION__,#_cond)