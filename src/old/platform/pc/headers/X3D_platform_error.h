/* This file is part of X3D.
 * 
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef X3D_CORE_ERROR_C
#error This file is only to be included in util/debug/error.c
#endif

#include "X3D_interface.h"
#include "X3D_platform.h"


int16 x3d_error_setup_error_frame(X3D_ErrorFrame* frame) {
  frame->next = x3d->platform_data.error_data.head;
  x3d->platform_data.error_data.head = frame;
  
  return setjmp(frame->buf);
}

void x3d_error_do_throw(int16 code) {
  X3D_ErrorFrame* frame = x3d->platform_data.error_data.head;
  
  if(!frame) {
    printf("Uncaught error(%d): %s\n", code, x3d->error.msg);
    exit(0);
  }
  
  x3d->platform_data.error_data.head = frame->next;
  
  longjmp(frame->buf, code);
}