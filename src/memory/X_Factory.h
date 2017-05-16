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

#include <assert.h>
#include <stdlib.h>

#define X_FACTORY_INVALID_HANDLE -1

////////////////////////////////////////////////////////////////////////////////
/// Manages a list of allocations using the factory pattern. Every allocation
///     is given a unique handle for the lifetime of the allocated object
///     (handles may be reused once the object is freed).
///
/// @note The factory owns all of the allocations made. Do not try to free them
///     directly!
/// @note A handle is an index into the allocation list.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_Factory
{
    void** allocList;               ///< List of all allocations made (handle slots)
    int allocListSize;              ///< Size of the allocation pool
    void** handleFreeListHead;      ///< Pointer to next available handle slot
    int handlesToAddOnResize;       ///< Number of new handle slots to add if none are available
} X_Factory;

void x_factory_init(X_Factory* factory, int initialHandlePoolSize, int handlesToAddOnResize);
void x_factory_cleanup(X_Factory* factory);

void x_factory_add_handles(X_Factory* factory, int handlesToAdd);
void* x_factory_alloc(X_Factory* factory, size_t allocSize, int* handleDest);
void x_factory_free(X_Factory* factory, int handle);

int x_factory_get_handle_from_alloc_ptr(const X_Factory* factory, void* ptr);

////////////////////////////////////////////////////////////////////////////////
/// Gets the pointer to an allocation from its handle.
////////////////////////////////////////////////////////////////////////////////
static inline void* x_factory_get_alloc_ptr_from_handle(const X_Factory* factory, int handle)
{
    assert(handle < factory->allocListSize);
    return factory->allocList[handle];
}

