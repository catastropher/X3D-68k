#include <tigcclib.h>

#define _GENERIC_ARCHIVE

#include <factorial.h>

long factorial(long input) {
  if (input < 1)
    return 1;
  else
    return input * factorial(input - 1);
}