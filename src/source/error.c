// C Source File
// Created 3/9/2015; 11:23:36 AM

#include "X3D_config.h"

#ifndef NDEBUG

// Throws an error, prints out the message, and then quits the program
void x3d_error(const char* format, ...) {
  char buf[512];
  va_list list;

  va_start(list, format);
  vsprintf(buf, format, list);

  PortRestore();
  clrscr();
  printf("Error: %s\nPress Esc to quit\n", buf);

  while(!_keytest(RR_ESC));

  exit(-1);
}

#endif

