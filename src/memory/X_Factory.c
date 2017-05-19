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

#include "X_Factory.h"
#include "memory/X_alloc.h"

////////////////////////////////////////////////////////////////////////////////
/// Initializes a factory.
///
/// @param factory                  - factory to initialize
/// @param initialHandlePoolSize    - initial size of the object list (in handles)
/// @param handlesToAddOnResize     - number of handles to add if we run out
///
/// @note This allocates memory to maintain a list of all allocated object. Make
///     sure to call @ref x_factory_cleanup() when done!
////////////////////////////////////////////////////////////////////////////////
void x_factory_init(X_Factory* factory, int initialHandlePoolSize, int handlesToAddOnResize)
{
    assert(handlesToAddOnResize >= 1);
    
    factory->handlesToAddOnResize = handlesToAddOnResize;
    factory->allocList = NULL;
    factory->allocListSize = 0;
    factory->handleFreeListHead = NULL;
    
    x_factory_add_handles(factory, initialHandlePoolSize);
}

// Releases an allocated object
static inline void x_factory_release_object(void** handleSlot)
{
    if(!*handleSlot)
        return;
    
    x_free(*handleSlot);
    *handleSlot = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Cleans up a factory and releases every object it had allocated (that wasn't
///     already freed).
////////////////////////////////////////////////////////////////////////////////
void x_factory_cleanup(X_Factory* factory)
{
    // All of the free slots have a pointer to the next free slot, which we'd
    // confuse as memory to free. So, follow the list of free slots and change
    // them to NULL.
    void** next = factory->handleFreeListHead;
    
    do
    {
        next = (void **)(*next);
        *factory->handleFreeListHead = NULL;
        factory->handleFreeListHead = next;
    } while(next);
    
    for(int i = 0; i < factory->allocListSize; ++i)
    {
        x_factory_release_object(factory->allocList + i);
    }
    
    x_free(factory->allocList);
    factory->allocList = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Adds additional handles to a factory. Should be used if several new
///     allocations are expected (for efficieny).
////////////////////////////////////////////////////////////////////////////////
void x_factory_add_handles(X_Factory* factory, int handlesToAdd)
{
    if(handlesToAdd == 0)
        return;
    
    // Add new handles
    int first_new_handle = factory->allocListSize;
    factory->allocListSize += handlesToAdd;
    factory->allocList = x_realloc(factory->allocList, factory->allocListSize * sizeof(void *));
    
    // Add the new handles to the free list
    for(int i = first_new_handle; i < factory->allocListSize - 1; ++i)
    {
        factory->allocList[i] = &factory->allocList[i + 1];
    }
    
    factory->allocList[factory->allocListSize - 1] = factory->handleFreeListHead;    
    factory->handleFreeListHead = &factory->allocList[first_new_handle];    
}

////////////////////////////////////////////////////////////////////////////////
/// Allocates a new object that is owned by the factory.
///
/// @param factory      - factory to allocate from
/// @param allocSize    - size of the allocation
/// @param handleDest   - pointer to where to write the allocation's handle
///
/// @return Pointer to the allocated memory.
///
/// @note Do not free this allocation directly; use @ref x_factory_free()
/// @note If not freed directly and @ref x_factory_cleanup() is called, this
///     allocation will be freed and all references to it are invalid.
////////////////////////////////////////////////////////////////////////////////
void* x_factory_alloc(X_Factory* factory, size_t allocSize, int* handleDest)
{
    if(!factory->handleFreeListHead)
        x_factory_add_handles(factory, factory->handlesToAddOnResize);
    
    // Get the next available handle slot
    void** handleSlot = factory->handleFreeListHead;
    factory->handleFreeListHead = (void**)(*factory->handleFreeListHead);
    
    void* alloc = x_malloc(allocSize);
    *handleSlot = alloc;
    *handleDest = handleSlot - factory->allocList;
    
    return alloc;
}

////////////////////////////////////////////////////////////////////////////////
/// Frees an allocation by its handle (see @ref x_factory_alloc()).
////////////////////////////////////////////////////////////////////////////////
void x_factory_free(X_Factory* factory, int handle)
{
    assert(handle < factory->allocListSize);
    x_factory_release_object(factory->allocList + handle);
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the handle for an allocation from the pointer to its allocated data.
///
/// @return The handle if ptr is a valid allocation,
///     @ref X_FACTORY_INVALID_HANDLE otherwise.
////////////////////////////////////////////////////////////////////////////////
int x_factory_get_handle_from_alloc_ptr(const X_Factory* factory, void* ptr)
{
    for(int i = 0; i < factory->allocListSize; ++i)
    {
        if(factory->allocList[i] == ptr)
            return i;
    }
    
    return X_FACTORY_INVALID_HANDLE;
}

