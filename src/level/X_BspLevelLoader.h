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
#include "X_BspLevel.h"
#include "memory/X_StreamReader.hpp"
#include "memory/X_Array.hpp"
#include "engine/X_EngineQueue.hpp"

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

#define X_BSPFILE_MAX_LEAFS 8192
#define X_BSPFILE_PVS_SIZE ((X_BSPFILE_MAX_LEAFS + 7) / 8)

#define X_BSPLEAF_OUTSIDE_LEVEL 0

struct X_RenderContext;

typedef struct X_BspLoaderLump
{
    int fileOffset;
    int length;
} X_BspLoaderLump;

typedef struct X_BspLoaderMipTextureLump
{
    int totalMipTextures;
    int* mipTextureOffsets;
} X_BspLoaderMipTextureLump;

typedef struct X_BspLoaderHeader
{
    int bspVersion;
    X_BspLoaderLump lumps[X_BSP_TOTAL_LUMPS];
} X_BspLoaderHeader;


typedef enum X_BspLoaderPlaneType
{
    X_BSPPLANE_X = 0,
    X_BSPPLANE_Y = 1,
    X_BSPPLANE_Z = 2,
    X_BSPPLANE_ANY_X = 3,
    X_BSPPLANE_ANY_Y = 4,
    X_BSPPLANE_ANY_Z = 5
} X_BspLoaderPlaneType;

typedef struct X_BspLoaderPlane
{
    Vec3fp normal;
    fp d;

    X_BspLoaderPlaneType type;
} X_BspLoaderPlane;

typedef struct X_BspLoaderVertex
{
    Vec3fp v;
} X_BspLoaderVertex;

typedef struct X_BspLoaderEdge
{
    short v[2];
} X_BspLoaderEdge;

typedef struct X_BspLoaderTexture
{
    char name[16];
    int w;
    int h;
    unsigned int texelsOffset[X_BSPTEXTURE_MIP_LEVELS];
} X_BspLoaderTexture;

typedef struct X_BspLoaderFaceTexture
{
    Vec3fp uOrientation;    // Orientation of texture in 3D space
    Vec3fp vOrientation;
    fp uOffset;
    fp vOffset;
    int textureId;
    int flags;
} X_BspLoaderFaceTexture;

#define X_BSPFACE_MAX_LIGHTMAPS 4

typedef struct X_BspLoaderFace
{
    short planeNum;
    short side;
    int firstEdge;
    short totalEdges;
    short texInfo;
    
    unsigned char lightmapStyles[X_BSPFACE_MAX_LIGHTMAPS];
    int lightmapOffset;
} X_BspLoaderFace;

#define X_BSPLEAF_TOTAL_AMBIENTS 4

typedef struct X_BspLoaderLeaf
{
    int contents;
    int pvsOffset;
    
    short mins[3];
    short maxs[3];
    
    unsigned short firstMarkSurface;
    unsigned short totalMarkSurfaces;
    
    unsigned char ambientLevel[X_BSPLEAF_TOTAL_AMBIENTS];
} X_BspLoaderLeaf;

typedef struct X_BspLoaderNode
{
    int planeNum;
    short children[2];
    short mins[3];
    short maxs[3];
    unsigned short firstFace;
    unsigned short totalFaces;
} X_BspLoaderNode;

#define X_BSPMODEL_MAX_MAP_HULLS 4

typedef struct X_BspLoaderModel
{
    float mins[3];
    float maxs[3];
    Vec3f origin;
    int rootBspNode;
    int rootClipNode;
    int secondRootClipNode;
    int thirdRootClipNode;
    int totalBspLeaves;
    int firstFaceId;
    int totalFaces;
} X_BspLoaderModel;

typedef struct X_BspLevelLoader
{
    X_File file;
    X_BspLoaderHeader header;
    
    Array<X_BspLoaderVertex> vertices;
    Array<X_BspLoaderEdge> edges;
    Array<X_BspLoaderPlane> planes;
    Array<X_BspLoaderFace> faces;
    Array<X_BspLoaderLeaf> leaves;
    Array<X_BspLoaderNode> nodes;
    Array<X_BspClipNode> clipNodes;
    Array<X_BspLoaderModel> models;

    X_BspCollisionHull collisionHulls[X_BSPLEVEL_MAX_COLLISION_HULLS];
    
    X_Color* textureTexels;
    X_BspLoaderTexture* textures;
    int totalTextures;
    
    X_BspLoaderFaceTexture* faceTextures;
    int totalFaceTextures;
    
    Array<unsigned short> markSurfaces;

    Array<int> surfaceEdgeIds;
    
    Array<unsigned char> compressedPvsData;       // Potential visibility set
    
    Array<unsigned char> lightmapData;
    
    char* entityDictionary;

    template<typename T>
    void loadLump(int lumpId, Array<T>& dest, const char* name);

    EngineQueue* engineQueue;

    void sendProgressEvent()
    {
        engineQueue->addEvent(progressEvent);
        engineQueue->flush();
    }

    EngineEvent progressEvent;
} X_BspLevelLoader;

bool x_bsplevel_load_from_bsp_file(X_BspLevel* level, const char* fileName, EngineQueue* engineQueue);

