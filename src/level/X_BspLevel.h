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

#include "geo/X_Vec3.h"
#include "render/X_Texture.h"
#include "geo/X_Plane.h"

struct X_RenderContext;

typedef int X_BspVertexId;
typedef int X_BspEdgeId;
typedef int X_BspLeafId;

typedef struct X_BspBoundBox
{
    X_Vec3_short min;
    X_Vec3_short max;
} X_BspBoundBox;

typedef struct X_BspVertex
{
    X_Vec3 v;
} X_BspVertex;

typedef struct X_BspFace
{
    
} X_BspFace;

typedef struct X_BspEdge
{
    unsigned short v[2];
} X_BspEdge;

typedef struct X_BspPlane
{
    X_Plane plane;
} X_BspPlane;

typedef enum X_BspLeafContents
{
    X_BSPLEAF_NODE = 0,
    X_BSPLEAF_REGULAR = -1,
    X_BSPLEAF_SOLID = -2,
    X_BSPLEAF_WATER = -3,
    X_BSPLEAF_SLIME = -4,
    X_BSPLEAF_LAVA = -5,
    X_BSPLEAF_SKY = -6
} X_BspLeafContents;

struct X_BspNode;

typedef struct X_BspNode
{
    // Common with X_BspLeaf - DO NOT REORDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    X_BspBoundBox boundBox;
    struct X_BspNode* parent;
    
    // Unique elements for node
    X_BspPlane* plane;
    struct X_BspNode* frontChild;
    struct X_BspNode* backChild;
} X_BspNode;

typedef struct X_BspLeaf
{
    // Common with X_BspNode - DO NOT REOREDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    X_BspBoundBox boundBox;
    struct X_BspNode* parent;
    
    // Unique elements for leaf
    unsigned char* compressedPvsData;
    
} X_BspLeaf;

typedef enum X_BspLevelFlags
{
    X_BSPLEVEL_LOADED = 1
} X_BspLevelFlags;

typedef struct X_BspModel
{
    X_BspBoundBox boundBox;
    X_BspNode* rootBspNode;
    int totalBspLeaves;
    
    // TODO: add clip node
    X_BspFace* faces;
    int totalFaces;
} X_BspModel;

typedef struct X_BspLevel
{
    X_BspLevelFlags flags;
    
    X_BspVertex* vertices;
    int totalVertices;
    
    X_BspEdge* edges;
    int totalEdges;
    
    X_BspFace* faces;
    int totalFaces;
    
    X_BspLeaf* leaves;
    int totalLeaves;
    
    X_BspNode* nodes;
    int totalNodes;
    
    X_BspModel* models;
    int totalModels;
    
    X_BspPlane* planes;
    int totalPlanes;
    
    unsigned char* compressedPvsData;
} X_BspLevel;

void x_bsplevel_render_wireframe(X_BspLevel* level, struct X_RenderContext* rcontext, X_Color color);
void x_bsplevel_init_empty(X_BspLevel* level);
X_BspLeaf* x_bsplevel_find_leaf_point_is_in(X_BspLevel* level, X_Vec3* point);

//void x_bsplevel_mark_leaves_from_pvs(X_BspLevel* level, unsigned char* pvs, int currentFrame);
void x_bsplevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLeaf* leaf, unsigned char* decompressedPvsDest);
int x_bsplevel_count_visible_leaves(X_BspLevel* level, unsigned char* pvs);

static inline _Bool x_bsplevel_file_is_loaded(const X_BspLevel* level)
{
    return (level->flags & X_BSPLEVEL_LOADED) != 0;
}

static inline X_BspLeaf* x_bsplevel_get_leaf(const X_BspLevel* level, X_BspLeafId leafId)
{
    return level->leaves + leafId;
}

static inline X_BspModel* x_bsplevel_get_level_model(const X_BspLevel* level)
{
    return level->models + 0;
}

static inline _Bool x_bspnode_is_leaf(const X_BspNode* node)
{
    return node->contents < 0;
}


static inline X_BspNode* x_bsplevel_get_root_node(const X_BspLevel* level)
{
    return x_bsplevel_get_level_model(level)->rootBspNode;
}
 
static inline int x_bspfile_node_pvs_size(const X_BspLevel* level)
{
    return (x_bsplevel_get_level_model(level)->totalBspLeaves + 7) / 8;
}

