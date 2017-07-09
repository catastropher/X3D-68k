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

#include "X_alloc.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "util/X_util.h"

typedef struct X_MemoryAlloc
{
    size_t size;
    struct X_MemoryAlloc* next;
    struct X_MemoryAlloc* prev;
    const char* fileName;

    const char* functionName;
    int lineNumber;
} X_MemoryAlloc;

static X_MemoryAlloc allocHead;
static X_MemoryAlloc allocTail;

static size_t totalMemoryUsage;
static size_t maxMemoryUsage;

void x_memory_init(void)
{
    allocHead.next = &allocTail;
    allocHead.prev = NULL;
    
    allocTail.next = NULL;
    allocTail.prev = &allocHead;
}

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
void* x_malloc_function(size_t size, const char* fileName, const char* functionName, int lineNumber)
{
    unsigned char* mem = malloc(size + sizeof(X_MemoryAlloc));
    
    if(!mem)
        x_system_error("Out of memory");
        
    
    X_MemoryAlloc* alloc = (X_MemoryAlloc*)mem;
    alloc->next = allocHead.next;
    alloc->next->prev = alloc;
    
    alloc->prev = &allocHead;
    alloc->prev->next = alloc;
    
    alloc->size = size;
    
    alloc->fileName = fileName;
    alloc->functionName = functionName;
    alloc->lineNumber = lineNumber;    
    
    totalMemoryUsage += size;
    maxMemoryUsage = X_MAX(maxMemoryUsage, totalMemoryUsage);
    
    return mem + sizeof(X_MemoryAlloc);
}

////////////////////////////////////////////////////////////////////////////////
/// Frees memory allocated by X3D's allocators.
////////////////////////////////////////////////////////////////////////////////
void x_free(void* mem)
{
    if(!mem)
        return;
    
    X_MemoryAlloc* alloc = (X_MemoryAlloc*)((unsigned char*)mem - sizeof(X_MemoryAlloc));

    totalMemoryUsage -= alloc->size;
    
    alloc->prev->next = alloc->next;
    alloc->next->prev = alloc->prev;
    
    free(alloc);
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
void* x_realloc_function(void* ptr, size_t newSize, const char* fileName, const char* function, int lineNumber)
{
    if(ptr == NULL)
        return x_malloc_function(newSize, fileName, function, lineNumber);
    
    X_MemoryAlloc* oldAlloc = (X_MemoryAlloc*)((unsigned char *)ptr - sizeof(X_MemoryAlloc));
    
    totalMemoryUsage += newSize - oldAlloc->size;
    maxMemoryUsage = X_MAX(maxMemoryUsage, totalMemoryUsage);
    
    X_MemoryAlloc* newAlloc = realloc(oldAlloc, newSize + sizeof(X_MemoryAlloc));
    
    if(!newAlloc)
        x_system_error("Realloc failed: out of memory");
    
    // We moved, so update the prev pointer
    newAlloc->prev->next = newAlloc;
    newAlloc->next->prev = newAlloc;
    
    newAlloc->size = newSize;
    
    return (unsigned char*)newAlloc + sizeof(X_MemoryAlloc);
}

void x_memory_free_all(void)
{
    X_MemoryAlloc* alloc = allocHead.next;
    
    x_log("Max memory usage during runtime: %d bytes (%d kb)", (int)maxMemoryUsage, (int)(maxMemoryUsage + 1023) / 1024);
    
    if(alloc == &allocTail)
    {
        x_log("No memory leaks detected :)");
        return;
    }
    
    x_log("Summary of memory leaks:");
    
    while(alloc != &allocTail)
    {
        X_MemoryAlloc* next = alloc->next;
        
        x_log("%d bytes allocated in %s (file %s, line %d)", (int)alloc->size, alloc->functionName, alloc->fileName, alloc->lineNumber);
        
        x_free((unsigned char*)alloc + sizeof(X_MemoryAlloc));
        alloc = next;
    }
}

