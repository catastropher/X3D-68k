#pragma once

#if defined(__CYGWIN__)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define _keytest(...) 1
#define ngetchx() ;
#define clrscr() ;

#endif

#ifdef __TIGCC_HEADERS__

#include <tigcclib.h>

#endif
