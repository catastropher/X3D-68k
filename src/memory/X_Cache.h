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

#include <stdlib.h>

typedef struct X_CacheEntry
{
    void* cacheData;
} X_CacheEntry;

typedef enum X_CacheBlockFlags
{
    X_CACHEBLOCK_FREE = 1
} X_CacheBlockFlags;

typedef struct X_CacheBlock
{
    size_t size;
    X_CacheBlockFlags flags;
    
    struct X_CacheBlock* next;
    struct X_CacheBlock* prev;
    
    struct X_CacheBlock* lruNext;
    struct X_CacheBlock* lruPrev;
    
    X_CacheEntry* cacheEntry;
    
    unsigned char memory[];
} X_CacheBlock;

typedef struct X_Cache
{
    const char* name;
    
    X_CacheBlock head;
    X_CacheBlock tail;
    
    void* cacheMem;
    size_t cacheSize;
} X_Cache;

void x_cache_init(X_Cache* cache, size_t size, const char* name);
void x_cache_cleanup(X_Cache* cache);

void x_cache_alloc(X_Cache* cache, size_t size, X_CacheEntry* entryDest);
void* x_cache_get_cached_data(X_Cache* cache, X_CacheEntry* entry);

static inline _Bool x_cachentry_is_in_cache(const X_CacheEntry* entry)
{
    return entry->cacheData != NULL;
}

