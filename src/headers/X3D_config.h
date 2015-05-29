#pragma once

#ifdef __CYGWIN32__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define _keytest(...) 1
#define ngetchx() ;
#define clrscr() ;

#endif

#ifdef __TIGCC

#include <tigcclib.h>

#endif
