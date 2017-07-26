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

#include "X_Cache.h"
#include "X_alloc.h"
#include "error/X_log.h"

void x_cache_init(X_Cache* cache, size_t size, const char* name)
{
    cache->name = name;
    cache->cacheSize = size;
    cache->cacheMem = x_malloc(size);
    
    // Create one giant memory block out of all of cache
    X_CacheBlock* block = (X_CacheBlock*)cache->cacheMem;
    
    cache->head.next = block;
    cache->head.prev = NULL;
    cache->head.lruNext = block;
    cache->head.lruPrev = NULL;
    
    block->next = &cache->tail;
    block->prev = &cache->head;
    block->lruNext = &cache->tail;
    block->lruPrev = &cache->head;
    block->size = size - sizeof(X_CacheBlock);
    
    cache->tail.next = NULL;
    cache->tail.prev = block;
    cache->tail.lruNext = NULL;
    cache->tail.lruPrev = block;
    
    x_log("Created cache %s (size = %d bytes)", name, (int)size);
}

void x_cache_cleanup(X_Cache* cache)
{
    x_free(cache->cacheMem);
}

static void x_cacheblock_split(X_CacheBlock* block, size_t size)
{
    // Don't bother splitting if the new block wouldn't even be big enough to hold a cache block
    if(block->size - size <= sizeof(X_CacheBlock))
        return;
    
    X_CacheBlock* newBlock = (X_CacheBlock*)((unsigned char*)block + sizeof(X_CacheBlock));
}

static X_CacheBlock* x_cache_try_alloc(X_Cache* cache, size_t size)
{
    // Round size up to the nearest multiple of 8 (for mem alignment)
    size = (size + 7) & (~7);
    
    for(X_CacheBlock* block = cache->head.nextFree; block != NULL; block = block->next)
    {
        if(x_cacheblock_is_free(block) && block->size >= size)
        {
            
        }
    }
}

