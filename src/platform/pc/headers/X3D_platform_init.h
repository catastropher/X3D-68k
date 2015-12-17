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
 
#ifndef X3D_UTIL_INIT_C
#error This file is only to be included in util/init/init.c
#endif
 
#include "X3D_interface.h"
#include "X3D_platform.h"

X3D_Interface* x3d;

static inline _Bool x3d_platform_init() {
  return X3D_TRUE;
}