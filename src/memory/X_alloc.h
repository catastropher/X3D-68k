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

////////////////////////////////////////////////////////////////////////////////
/// Allocates memory using X3D's own allocators.
///
/// @param size - size of the memory to allocate
///
/// @return A block of memory at lease size bytes big.
///
/// @note This will abort if there is not enough memory available.
/// @note Make sure to free this memory with @ref x_free() and NOT free().
////////////////////////////////////////////////////////////////////////////////
static inline void* x_malloc(size_t size)
{
    void* mem = malloc(size);
    if(!mem)
    {
        fprintf(stderr, "Out of memory (request %zu bytes)\n", size);
        abort();
    }
    
    return mem;
}

////////////////////////////////////////////////////////////////////////////////
/// Expands or shrinks a block of allocated memory.
///
/// @param ptr      - Memory previous allocated using e.g. @ref x_malloc()
/// @param newSize  - New size of the allocated memory
///
/// @return A pointer to the resized memory block.
///
/// @todo This should throw an error if realloc fails.
////////////////////////////////////////////////////////////////////////////////
static inline void* x_realloc(void* ptr, size_t newSize)
{
    return realloc(ptr, newSize);
}

////////////////////////////////////////////////////////////////////////////////
/// Frees memory allocated by X3D's allocators.
////////////////////////////////////////////////////////////////////////////////
static inline void x_free(void* mem)
{
    free(mem);
}

