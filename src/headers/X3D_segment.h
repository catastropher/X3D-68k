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
#include "memory/X3D_varsizeallocator.h"
#include "X3D_prism.h"

#define X3D_SEGMENT_NONE 0xFFFF
#define X3D_FACE_NONE 0xFFFF

#define X3D_SEGMENT_CACHE_SIZE 32

typedef uint16 X3D_SegFaceID;

typedef enum {
  X3D_SEGMENT_IN_CACHE = (1 << 15),
  X3D_SEGMENT_UNCOMPRESSED = (1 << 14)
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

typedef struct X3D_UncompressedSegmentFace {
  X3D_SegFaceID portal_seg_face;  ///< Face ID that the portal on the face is
                                  /// connected to
} X3D_UncompressedSegmentFace;

typedef struct X3D_SegmentBase {
  uint16 flags;
  uint16 id;
} X3D_SegmentBase;

typedef struct X3D_UncompressedSegment {
  X3D_SegmentBase base;
  X3D_Prism3D prism;
  uint16 face_offset;
} X3D_UncompressedSegment;

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
  X3D_VarSizeAllocator alloc;
  
  // Temorary store for segments
  X3D_Segment segments[];
} X3D_SegmentManager;

X3D_INTERNAL void x3d_segmentmanager_init(uint16 max_segments, uint16 seg_pool_size);
X3D_SegmentBase* x3d_segmentmanager_add(uint16 size);
X3D_INTERNAL X3D_Segment* x3d_segmentmanager_get_internal(uint16 id);

static inline uint16 x3d_uncompressedsegment_face_offset(uint16 base_v) {
  return sizeof(X3D_UncompressedSegment) + 2 * base_v * sizeof(X3D_Vex3D);
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the size needed to store an uncompressed segment with the
///   given number of vertices in one of the prism's bases.
///
/// @param base_v - number of vertices in the prism base
///
/// @return Size in bytes.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_uncompressedsegment_size(uint16 base_v) {
  return x3d_uncompressedsegment_face_offset(base_v) +
    x3d_prism3d_total_f(base_v) * sizeof(X3D_UncompressedSegmentFace);
}

static inline X3D_UncompressedSegmentFace* x3d_uncompressedsegment_get_faces(X3D_UncompressedSegment* seg) {
  return ((void *)seg) + seg->face_offset;
}


