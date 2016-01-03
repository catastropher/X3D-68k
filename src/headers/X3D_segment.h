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

#include "X3D_common.h"
#include "X3D_prism.h"

#define X3D_SEGMENT_CACHE_SIZE 32

#define X3D_MAX_SEGMENTS 32

typedef enum {
  X3D_SEGMENT_IN_CACHE = 1
} X3D_SegmentFlags;

///////////////////////////////////////////////////////////////////////////////
/// A segment, the basic unit that levels are made of. Levels are made of
///   interconnected segments that share a face. Each segment is a 3D prism (
///   see @ref X3D_Prism3D).
///
/// @note This structure should not be accesed directly, as the level
///   geometry is stored in a compressed format. It should be loaded into the
///   segment cache first.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Segment {
  uint16 flags;
  
  X3D_Prism3D prism;
} X3D_Segment;

typedef struct X3D_SegmentCacheEntry {
  uint8 prev;
  uint8 next;
  X3D_Prism3D prism;
} X3D_SegmentCacheEntry;

typedef struct X3D_SegmentCache {
  X3D_SegmentCacheEntry entry[X3D_SEGMENT_CACHE_SIZE];
  uint16 lru_head;  ///< Index of the least recently used (LRU) cache entry
  uint16 lru_tail;  ///< Most-recently used cache entry
} X3D_SegmentCache;

typedef struct X3D_SegmentManager {
  X3D_SegmentCache cache;
  
  // Temorary store for segments
  X3D_Segment segments[];
} X3D_SegmentManager;

