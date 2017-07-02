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
#include "geo/X_Plane.h"
#include "render/X_Texture.h"
#include "system/X_File.h"

#define X_LUMP_ENTITIES     0
#define X_LUMP_PLANES       1
#define X_LUMP_TEXTURES     2
#define X_LUMP_VERTEXES     3
#define X_LUMP_VISIBILITY   4
#define X_LUMP_NODES        5
#define X_LUMP_TEXINFO      6
#define X_LUMP_FACES        7
#define X_LUMP_LIGHTING     8
#define X_LUMP_CLIPNODES    9
#define X_LUMP_LEAFS        10
#define X_LUMP_MARKSURFACES 11
#define X_LUMP_EDGES        12
#define X_LUMP_SURFEDGES    13
#define X_LUMP_MODELS       14

#define X_BSP_TOTAL_LUMPS 15

typedef int X_BspVertexId;
typedef int X_BspEdgeId;

struct X_RenderContext;

typedef struct X_BspLump
{
    int fileOffset;
    int length;
} X_BspLump;

typedef struct X_BspHeader
{
    int bspVersion;
    X_BspLump lumps[X_BSP_TOTAL_LUMPS];
} X_BspHeader;


typedef enum X_BspPlaneType
{
    X_BSPPLANE_X = 0,
    X_BSPPLANE_Y = 1,
    X_BSPPLANE_Z = 2,
    X_BSPPLANE_ANY_X = 3,
    X_BSPPLANE_ANY_Y = 4,
    X_BSPPLANE_ANY_Z = 5
} X_BspPlaneType;

typedef struct X_BspPlane
{
    X_Plane plane;
    X_BspPlaneType type;
} X_BspPlane;

typedef struct X_BspVertex
{
    X_Vec3 v;
} X_BspVertex;

typedef struct X_BspEdge
{
    X_BspVertexId v[2];
} X_BspEdge;

#define X_BSPFACE_MAX_LIGHTMAPS 4

typedef struct X_BspFace
{
    short planeNum;
    short side;
    X_BspEdgeId firstEdge;
    short totalEdges;
    short texInfo;
    
    unsigned char lightmapStypes[X_BSPFACE_MAX_LIGHTMAPS];
    int lightmapOffset;
} X_BspFace;

#define X_BSPLEAF_TOTAL_AMBIENTS 4

typedef struct X_BspLeaf
{
    int contents;
    unsigned char* compressedPvsData;
    
    short mins[3];
    short maxs[3];
    
    unsigned short firstMarkSurface;
    unsigned short numMarkSurface;
    
    unsigned char ambientLevel[X_BSPLEAF_TOTAL_AMBIENTS];
} X_BspLeaf;

typedef struct X_BspNode
{
    int planeNum;
    short children[2];
    short mins[3];
    short maxs[3];
    unsigned short firstFace;
    unsigned short totalFaces;
} X_BspNode;

typedef struct X_BspLevel
{
    X_File file;
    X_BspHeader header;
    
    X_BspVertex* vertices;
    int totalVertices;
    
    X_BspEdge* edges;
    int totalEdges;
    
    X_BspPlane* planes;
    int totalPlanes;
    
    X_BspFace* faces;
    int totalFaces;
    
    X_BspLeaf* leaves;
    int totalLeaves;
    
    X_BspNode* nodes;
    int totalNodes;
    
    unsigned char* compressedPvsData;       // Potential visibility set
} X_BspLevel;

_Bool x_bsplevel_load_from_bsp_file(X_BspLevel* level, const char* fileName);
void x_bsplevel_render_wireframe(X_BspLevel* level, struct X_RenderContext* rcontext, X_Color color);
int x_bsplevel_find_leaf_point_is_in(X_BspLevel* level, int nodeId, X_Vec3* point);
void x_bsplevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLeaf* leaf, unsigned char* decompressedPvsDest);

static inline int x_bspfile_node_pvs_size(const X_BspLevel* level)
{
    return (level->totalLeaves + 8 / 2) / 8;
}

