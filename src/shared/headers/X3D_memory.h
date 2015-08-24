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

void* x3d_malloc_function(uint16 size, const char* function_name);
void x3d_free_function(void* mem, void* function_name);
void x3d_free_all();
void x3d_init_memorymanager();

#define x3d_malloc(_size) x3d_malloc_function(_size, __FUNCTION__)
#define x3d_free(_mem) x3d_free_function(_mem, __FUNCTION__)

