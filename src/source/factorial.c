#include <tigcclib.h>
#include <factorial.h>

#define _GENERIC_ARCHIVE

long factorial(long input) {
  if (input < 1)
    return 1;
  else
    return input * factorial(input - 1);
}