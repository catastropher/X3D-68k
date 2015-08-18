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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_engine.h"

#ifdef __X3D_SHARED__

void* x3d_malloc(X3D_Context* context, uint16 size);
void x3d_free(X3D_Context* context, void* ptr);
void* x3d_realloc(X3D_Context* context, void* ptr, uint16 size);

#else

#define x3d_malloc _DLL_call(void* , (X3D_Context* context, uint16 size), EXPORT_X3D_MALLOC)
#define x3d_free _DLL_call(void, (X3D_Context* context, void* ptr), EXPORT_X3D_FREE)
#define x3d_realloc _DLL_call(void* , (X3D_Context* context, void* ptr, uint16 size), EXPORT_X3D_REALLOC)

#endif

