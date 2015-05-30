#pragma once

#ifdef _MSC_VER
#define inline __inline
#endif

#if defined(__CYGWIN__) || defined(WIN32)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#define _keytest(...) 1
#define ngetchx() ;
#define clrscr() ;

#endif

#ifdef __TIGCC_HEADERS__

#include <tigcclib.h>

#endif
