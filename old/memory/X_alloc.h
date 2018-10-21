// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <stdlib.h>
#include <stdio.h>

#define x_malloc(_size) x_malloc_function(_size, __FILE__, __FUNCTION__, __LINE__)
#define x_realloc(_ptr, _newSize) x_realloc_function(_ptr, _newSize, __FILE__, __FUNCTION__, __LINE__)

void* x_malloc_function(size_t size, const char* fileName, const char* functionName, int lineNumber);
void x_free(void* mem);
void* x_realloc_function(void* ptr, size_t newSize, const char* fileName, const char* function, int lineNumber);
void x_memory_init(void);
void x_memory_free_all(void);

