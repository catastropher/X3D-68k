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

#include "X3D_config.h"
#include "X3D_fix.h"

#ifndef NDEBUG

// Throws an error, prints out the message, and then quits the program
void x3d_error(const char* format, ...) {
  char buf[512];
  va_list list;

  va_start(list, format);
  vsprintf(buf, format, list);

#ifdef __TIGCC__
  PortRestore();
  clrscr();

#endif
  printf("Error: %s\nPress Esc to quit\n", buf);

#ifdef __TIGCC__
  while(!_keytest(RR_ESC));
#endif

  exit(-1);
}

#endif

