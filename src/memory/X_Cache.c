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
#include "error/X_error.h"

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
    cache->head.flags = 0;
    
    block->next = &cache->tail;
    block->prev = &cache->head;
    block->lruNext = &cache->tail;
    block->lruPrev = &cache->head;
    block->size = size - sizeof(X_CacheBlock);
    block->flags = X_CACHEBLOCK_FREE;
    
    cache->tail.next = NULL;
    cache->tail.prev = block;
    cache->tail.lruNext = NULL;
    cache->tail.lruPrev = block;
    cache->tail.flags = 0;
    
    x_log("Created cache %s (size = %d bytes)", name, (int)size);
}

void x_cache_cleanup(X_Cache* cache)
{
    x_free(cache->cacheMem);
}

static _Bool x_cacheblock_is_free(X_CacheBlock* block)
{
    return block->flags & X_CACHEBLOCK_FREE;
}


static void x_cache_mark_block_as_least_recently_used(X_Cache* cache, X_CacheBlock* block)
{
    block->lruNext = cache->head.lruNext;
    cache->head.lruNext->lruPrev = block;
    
    cache->head.lruNext = block;
    block->lruPrev = &cache->head;
}

static void x_cacheblock_unlink_lru(X_CacheBlock* block)
{
    block->lruPrev->lruNext = block->lruNext;
    block->lruNext->lruPrev = block->lruPrev;
    
    block->lruPrev = NULL;
    block->lruNext = NULL;
}

static void x_cache_mark_block_as_most_recently_used(X_Cache* cache, X_CacheBlock* block)
{
    x_cacheblock_unlink_lru(block);
    
    block->lruNext = &cache->tail;
    cache->tail.lruPrev = block;
    
    X_CacheBlock* oldMostRecent = cache->tail.lruPrev;
    oldMostRecent->lruNext = block;
    block->lruPrev = oldMostRecent;
}

static void x_cacheblock_insert_after(X_CacheBlock* blockToInsertAfter, X_CacheBlock* blockToInsert)
{
    blockToInsert->next = blockToInsertAfter->next;
    blockToInsertAfter->prev = blockToInsert;
    
    blockToInsert->prev = blockToInsertAfter;
    blockToInsertAfter->next = blockToInsert;
}

static void x_cacheblock_split(X_CacheBlock* block, size_t size)
{
    // Don't bother splitting if the new block wouldn't even be big enough to hold a cache block
    if(block->size - size <= sizeof(X_CacheBlock))
        return;
    
    X_CacheBlock* newBlock = (X_CacheBlock*)((unsigned char*)block + sizeof(X_CacheBlock));
    newBlock->size = block->size - size - sizeof(X_CacheBlock);
    x_cacheblock_insert_after(block, newBlock);
    
    block->size -= size + sizeof(X_CacheBlock);
}

static void x_cacheblock_merge_with_next(X_CacheBlock* block)
{
    x_assert(block->next == (X_CacheBlock*)((unsigned char*)block + sizeof(X_CacheBlock) + block->size), "Attempting to merge non-consecutive cache blocks");
    x_assert(x_cacheblock_is_free(block) && x_cacheblock_is_free(block->next), "Trying to merge non-free blocks");
    
    block->size += block->next->size + sizeof(X_CacheBlock);
    
    X_CacheBlock* next = block->next;
    next->next->prev = block;
    block->next = next->next;
}

static X_CacheBlock* x_cache_try_alloc(X_Cache* cache, size_t size)
{
    // Round size up to the nearest multiple of 8 (for mem alignment)
    size = (size + 7) & (~7);
    
    for(X_CacheBlock* block = cache->head.next; block != NULL; block = block->next)
    {
        if(x_cacheblock_is_free(block) && block->size >= size)
        {
            x_cacheblock_split(block, size);
            x_cache_mark_block_as_least_recently_used(cache, block);
            
            return block;
        }
    }
    
    return NULL;
}

static void x_cacheblock_free(X_CacheBlock* block)
{
    block->cacheEntry->cacheData = NULL;
    
    if(x_cacheblock_is_free(block->prev))
    {
        x_cacheblock_unlink_lru(block);
        block = block->prev;
        x_cacheblock_merge_with_next(block);
    }
    
    if(x_cacheblock_is_free(block->next))
    {
        x_cacheblock_unlink_lru(block);
        x_cacheblock_merge_with_next(block);
    }
}

static _Bool x_cache_free_least_recently_used_block(X_Cache* cache)
{
    X_CacheBlock* blockToFree = cache->head.lruNext;
    
    if(blockToFree == &cache->tail)
        return 0;
    
    cache->head.lruNext = blockToFree->lruNext;
    blockToFree->lruNext->lruPrev = &cache->head;
    
    x_cacheblock_free(blockToFree);
    return 1;
}

void x_cache_alloc(X_Cache* cache, size_t size, X_CacheEntry* entryDest)
{
    do
    {
        X_CacheBlock* newBlock = x_cache_try_alloc(cache, size);
        
        if(newBlock)
        {
            entryDest->cacheData = newBlock;
            return;
        }
    } while(x_cache_free_least_recently_used_block(cache));
    
    x_system_error("Cache %s too small for allocation of %d bytes", cache->name, (int)size);
}

void* x_cache_get_cached_data(X_Cache* cache, X_CacheEntry* entry)
{
    if(entry->cacheData == NULL)
        return NULL;
    
    X_CacheBlock* block = (X_CacheBlock*)entry->cacheData;
    x_cache_mark_block_as_most_recently_used(cache, block);
    
    return (void*)block->memory;
}

