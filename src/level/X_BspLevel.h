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

#include "geo/X_Polygon3.h"
#include "geo/X_Polygon2.hpp"
#include "geo/X_Vec3.h"
#include "render/X_Texture.h"
#include "geo/X_Plane.h"
#include "util/X_util.h"
#include "memory/X_Cache.h"
#include "render/X_Light.h"
#include "geo/X_BoundBox.h"
#include "memory/X_Link.h"
#include "math/X_Mat4x4.h"

struct X_RenderContext;
struct X_AE_Edge;

typedef int X_BspVertexId;
typedef int X_BspEdgeId;
typedef int X_BspLeafId;

#define X_BSPTEXTURE_MIP_LEVELS 4

typedef struct X_BspTexture
{
    char name[16];
    unsigned int w;
    unsigned int h;
    unsigned char* mipTexels[X_BSPTEXTURE_MIP_LEVELS];
} X_BspTexture;

typedef struct X_BspFaceTexture
{
    Vec3 uOrientation;    // Orientation of texture in 3D space
    Vec3 vOrientation;
    x_fp16x16 uOffset;
    x_fp16x16 vOffset;
    X_BspTexture* texture;
    int flags;
} X_BspFaceTexture;

typedef struct X_BspBoundRect
{
    X_Vec2 v[2];
} X_BspBoundRect;

typedef struct X_BspPlane
{
    Plane plane;
} X_BspPlane;

typedef struct X_BspVertex
{
    Vec3 v;
    int cacheOffset;
} X_BspVertex;

typedef enum X_BspSurfaceFlags
{
    X_BSPSURFACE_FLIPPED = 1,
} X_BspSurfaceFlags;

#define X_BSPSURFACE_MAX_LIGHTMAPS 4

struct X_BspSurface
{
    void calculatePlaneInViewSpace(Vec3* camPos, Mat4x4* viewMatrix, Vec3* pointOnSurface, Plane* dest)
    {
        Vec3fp pointOnSurfaceTemp = MakeVec3fp(*pointOnSurface);
        Vec3fp camPosTemp = MakeVec3fp(*camPos);

        dest->normal = viewMatrix->transformNormal(plane->plane.normal);

        fp d = -plane->plane.normal.dot(pointOnSurfaceTemp);

        dest->d = d + plane->plane.normal.dot(camPosTemp);
    }
    
    int id;     // Just for debugging
    int lastVisibleFrame;
    X_BspPlane* plane;
    
    int firstEdgeId;
    int totalEdges;
    
    X_BspSurfaceFlags flags;
    
    X_Color color;
    X_BspFaceTexture* faceTexture;
    
    X_Vec2 textureMinCoord;
    X_Vec2 textureExtent;
    
    unsigned char* lightmapData;
    unsigned char lightmapStyles[X_BSPSURFACE_MAX_LIGHTMAPS];
    
    unsigned int lightsTouchingSurface;
    int lastLightUpdateFrame;
    
    X_CacheEntry cachedSurfaces[X_BSPTEXTURE_MIP_LEVELS];   // Cached surface for each mipmap level
};

typedef struct X_BspEdge
{
    unsigned short v[2];
    unsigned int cachedEdgeOffset;
} X_BspEdge;

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
    X_BoundBox nodeBoundBox;
    X_BoundBox geoBoundBox;
    struct X_BspNode* parent;
    
    // Unique elements for node
    X_BspPlane* plane;
    
    struct X_BspNode* frontChild;
    struct X_BspNode* backChild;
    
    X_BspSurface* firstSurface;
    int totalSurfaces;
} X_BspNode;

typedef struct X_BspLeaf
{
    // Common with X_BspNode - DO NOT REOREDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    X_BoundBox nodeBoundBox;
    X_BoundBox geoBoundBox;
    struct X_BspNode* parent;
    
    // Unique elements for leaf
    X_BspSurface** firstMarkSurface;
    int totalMarkSurfaces;
    int bspKey;
    
    unsigned char* compressedPvsData;
} X_BspLeaf;

typedef enum X_BspLevelFlags
{
    X_BSPLEVEL_LOADED = 1
} X_BspLevelFlags;

typedef struct X_BspClipNode
{
    int planeId;
    short frontChild;
    short backChild;
} X_BspClipNode;

typedef struct X_BspModel
{
    X_BoundBox boundBox;
    X_BspNode* rootBspNode;
    int clipNodeRoots[3];
    
    int totalBspLeaves;
    
    X_BspSurface* faces;
    int totalFaces;
    
    Vec3 origin;
    
    X_Link objectsOnModelHead;
    X_Link objectsOnModelTail;
} X_BspModel;

typedef struct X_BspCollisionHull
{
    short rootNode;
} X_BspCollisionHull;

#define X_BSPLEVEL_MAX_COLLISION_HULLS 4

#define X_BSPLEVEL_MAX_NAME_LENGTH 32

typedef struct X_BspLevel
{
    void getLevelPolygon(X_BspSurface* surface, Vec3* modelOrigin, LevelPolygon3* dest)
    {
        dest->edgeIds = surfaceEdgeIds + surface->firstEdgeId;
        dest->totalVertices = surface->totalEdges;
        
        for(int i = 0; i < surface->totalEdges; ++i)
        {   
            Vec3 v;
            
            int edgeId = dest->edgeIds[i];
            bool edgeIsFlipped = (edgeId < 0);
            
            if(!edgeIsFlipped)
                v = vertices[edges[edgeId].v[0]].v;
            else
                v = vertices[edges[-edgeId].v[1]].v;
            
            dest->vertices[i] = MakeVec3fp(v + *modelOrigin);
        }
    }
    
    X_BspLevelFlags flags;
    char name[X_BSPLEVEL_MAX_NAME_LENGTH];
    
    X_BspVertex* vertices;
    int totalVertices;
    
    X_BspEdge* edges;
    int totalEdges;
    
    X_BspSurface* surfaces;
    int totalSurfaces;
    
    X_BspSurface** markSurfaces;
    int totalMarkSurfaces;
    
    X_BspLeaf* leaves;
    int totalLeaves;
    
    X_BspNode* nodes;
    int totalNodes;
    
    X_BspClipNode* clipNodes;
    int totalClipNodes;
    
    X_BspCollisionHull collisionHulls[X_BSPLEVEL_MAX_COLLISION_HULLS];
    
    X_BspModel* models;
    int totalModels;
    
    X_BspPlane* planes;
    int totalPlanes;
    
    int* surfaceEdgeIds;
    int totalSurfaceEdgeIds;
    
    X_Color* textureTexels;
    X_BspTexture* textures;
    int totalTextures;
    
    X_BspFaceTexture* faceTextures;
    int totalFaceTextures;
    
    unsigned char* compressedPvsData;
    
    unsigned char* lightmapData;
    
    char* entityDictionary;
    
    int nextBspKey;
} X_BspLevel;

void x_bsplevel_cleanup(X_BspLevel* level);

void x_bsplevel_render_wireframe(X_BspLevel* level, struct X_RenderContext* rcontext, X_Color color);

void x_bsplevel_init_empty(X_BspLevel* level);
X_BspLeaf* x_bsplevel_find_leaf_point_is_in(X_BspLevel* level, Vec3* point);

void x_bsplevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLeaf* leaf, unsigned char* decompressedPvsDest);
int x_bsplevel_count_visible_leaves(X_BspLevel* level, unsigned char* pvs);
void x_bsplevel_mark_visible_leaves_from_pvs(X_BspLevel* level, unsigned char* pvs, int currentFrame);
void x_bsplevel_render(X_BspLevel* level, struct X_RenderContext* renderContext);

void x_bsplevel_get_texture(X_BspLevel* level, int textureId, int mipMapLevel, X_Texture* dest);

void x_bsplevel_mark_surfaces_light_is_close_to(X_BspLevel* level, const X_Light* light, int currentFrame);

struct X_BoundSphere;
int x_bsplevel_find_nodes_intersecting_sphere(X_BspLevel* level, struct X_BoundSphere* sphere, X_BspNode** dest);

void x_bsplevel_render_submodels(X_BspLevel* level, struct X_RenderContext* renderContext);

//======================== level ========================

static inline bool x_bsplevel_file_is_loaded(const X_BspLevel* level)
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

static inline X_BspNode* x_bsplevel_get_root_node(const X_BspLevel* level)
{
    return x_bsplevel_get_level_model(level)->rootBspNode;
}

static inline void x_bsplevel_reset_bspkeys(X_BspLevel* level)
{
    level->nextBspKey = 1;
}

static inline void x_bsplevel_next_bspkey(X_BspLevel* level)
{
    ++level->nextBspKey;
}

static inline int x_bsplevel_current_bspkey(const X_BspLevel* level)
{
    return level->nextBspKey;
}

static inline X_BspModel* x_bsplevel_get_model(X_BspLevel* level, int modelId)
{
    return level->models + modelId;
}

//======================== node ========================

static inline bool x_bspnode_is_leaf(const X_BspNode* node)
{
    return node->contents < 0;
}

static inline bool x_bspnode_is_visible_this_frame(const X_BspNode* node, int currentFrame)
{
    return node->lastVisibleFrame == currentFrame;
}

//======================== surface ========================

static inline bool x_bspsurface_is_visible_this_frame(const X_BspSurface* surface, int currentFrame)
{
    return surface->lastVisibleFrame == currentFrame;
}

static inline bool x_bspsurface_plane_is_flipped(const X_BspSurface* surface)
{
    return surface->flags & X_BSPSURFACE_FLIPPED;
}

//======================== pvs ========================

static inline int x_bspfile_node_pvs_size(const X_BspLevel* level)
{
    return (x_bsplevel_get_level_model(level)->totalBspLeaves + 7) / 8;
}

//======================== boundrect ========================

static inline void x_bspboundrect_init(X_BspBoundRect* rect)
{
    rect->v[0].x = 0x7FFFFFFF;
    rect->v[0].y = 0x7FFFFFFF;
    
    rect->v[1].x = -0x7FFFFFFF;
    rect->v[1].y = -0x7FFFFFFF;
}

static inline void x_bspboundrect_add_point(X_BspBoundRect* rect, X_Vec2 point)
{
    rect->v[0].x = X_MIN(rect->v[0].x, point.x);
    rect->v[0].y = X_MIN(rect->v[0].y, point.y);
    
    rect->v[1].x = X_MAX(rect->v[1].x, point.x);
    rect->v[1].y = X_MAX(rect->v[1].y, point.y);
}

//======================== model ========================

static inline x_fp16x16 x_bspmodel_height(X_BspModel* model)
{
    return model->boundBox.v[0].y - model->boundBox.v[1].y;
}

static inline bool x_bspmodel_has_objects_standing_on(X_BspModel* model)
{
    return model->objectsOnModelHead.next != &model->objectsOnModelTail;
}

