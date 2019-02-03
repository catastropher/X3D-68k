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

#include "geo/BoundBox.hpp"
#include "geo/Plane.hpp"
#include "geo/Polygon2.hpp"
#include "geo/Polygon3.hpp"
#include "geo/Vec3.hpp"
#include "level/BspModel.hpp"
#include "level/BspNode.hpp"
#include "math/Mat4x4.hpp"
#include "memory/Cache.h"
#include "memory/OldLink.hpp"
#include "render/Light.hpp"
#include "render/Texture.hpp"
#include "util/Util.hpp"
#include "PotentiallyVisibleSet.hpp"

struct X_RenderContext;
struct X_AE_Edge;
struct Portal;
struct X_BspLeaf;
struct BspModel;

typedef int X_BspVertexId;
typedef int X_BspEdgeId;
typedef int X_BspLeafId;

#define X_BSPTEXTURE_MIP_LEVELS 4

struct BspTexture
{
    char name[16];
    unsigned int w;
    unsigned int h;
    unsigned char* mipTexels[X_BSPTEXTURE_MIP_LEVELS];
};

struct BspFaceTexture
{
    Vec3fp uOrientation;    // Orientation of texture in 3D space
    Vec3fp vOrientation;
    x_fp16x16 uOffset;
    x_fp16x16 vOffset;
    BspTexture* texture;
    int flags;
};

typedef struct X_BspBoundRect
{
    Vec2 v[2];
} X_BspBoundRect;

struct BspPlane
{
    Plane plane;
};

struct BspVertex
{
    Vec3fp v;
};

typedef enum X_BspSurfaceFlags
{
    X_BSPSURFACE_FLIPPED = 1,
} X_BspSurfaceFlags;

#define X_BSPSURFACE_MAX_LIGHTMAPS 4

struct BspSurface
{
    void calculatePlaneInViewSpace(const Vec3fp& camPos, Mat4x4* viewMatrix, Vec3fp& pointOnSurface, Plane* dest)
    {
        dest->normal = viewMatrix->transformNormal(plane->plane.normal);

        fp d = -plane->plane.normal.dot(pointOnSurface);

        dest->d = d + plane->plane.normal.dot(camPos);
    }
    
    int id;     // Just for debugging
    int lastVisibleFrame;
    BspPlane* plane;
    
    int firstEdgeId;
    int totalEdges;
    
    X_BspSurfaceFlags flags;
    
    X_Color color;
    BspFaceTexture* faceTexture;
    
    Vec2 textureMinCoord;
    Vec2 textureExtent;
    
    unsigned char* lightmapData;
    unsigned char lightmapStyles[X_BSPSURFACE_MAX_LIGHTMAPS];
    
    unsigned int lightsTouchingSurface;
    int lastLightUpdateFrame;
    
    X_CacheEntry cachedSurfaces[X_BSPTEXTURE_MIP_LEVELS];   // Cached surface for each mipmap level
};

struct BspEdge
{
    unsigned short v[2];
    unsigned int cachedEdgeOffset;
};

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

typedef struct X_BspCollisionHull
{
    short rootNode;
} X_BspCollisionHull;

#define X_BSPLEVEL_MAX_COLLISION_HULLS 4

#define X_BSPLEVEL_MAX_NAME_LENGTH 32

struct BspLevel
{
    BspLevel()
        : pvs(*this)
    {
        
    }
    
    void initEmpty();

    X_BspLeaf* findLeafPointIsIn(Vec3fp& point);

    void getLevelPolygon(BspSurface* surface, Vec3fp* modelOrigin, LevelPolygon3* dest)
    {
        dest->edgeIds = surfaceEdgeIds + surface->firstEdgeId;
        dest->totalVertices = surface->totalEdges;
        
        for(int i = 0; i < surface->totalEdges; ++i)
        {   
            Vec3fp v;
            
            int edgeId = dest->edgeIds[i];
            bool edgeIsFlipped = (edgeId < 0);
            
            if(!edgeIsFlipped)
                v = vertices[edges[edgeId].v[0]].v;
            else
                v = vertices[edges[-edgeId].v[1]].v;
            
            dest->vertices[i] = v + *modelOrigin;
        }
    }

    Portal* addPortal();

    void renderPortals(X_RenderContext& renderContext);
    
    X_BspLevelFlags flags;
    char name[X_BSPLEVEL_MAX_NAME_LENGTH];
    
    BspVertex* vertices;
    int totalVertices;
    
    BspEdge* edges;
    int totalEdges;
    
    BspSurface* surfaces;
    int totalSurfaces;
    
    BspSurface** markSurfaces;
    int totalMarkSurfaces;
    
    X_BspLeaf* leaves;
    int totalLeaves;
    
    X_BspNode* nodes;
    int totalNodes;
    
    X_BspClipNode* clipNodes;
    int totalClipNodes;
    
    X_BspCollisionHull collisionHulls[X_BSPLEVEL_MAX_COLLISION_HULLS];
    
    BspModel* models;
    int totalModels;
    
    BspPlane* planes;
    int totalPlanes;
    
    int* surfaceEdgeIds;
    int totalSurfaceEdgeIds;
    
    X_Color* textureTexels;
    BspTexture* textures;
    int totalTextures;
    
    BspFaceTexture* faceTextures;
    int totalFaceTextures;
    
    PotentiallyVisibleSet pvs;
    
    unsigned char* lightmapData;
    
    char* entityDictionary;
    
    int nextBspKey;

    Portal* portalHead;     // Should be private
    
private:
    static void markAllLeavesInPvsAsVisible(unsigned char* pvs, int pvsSize);
    static void decompressPvs(unsigned char* compressedPvsData, int pvsSize, unsigned char* decompressedPvsDest);
};

void x_bsplevel_cleanup(BspLevel* level);

void x_bsplevel_render(BspLevel* level, struct X_RenderContext* renderContext);

void x_bsplevel_get_texture(BspLevel* level, int textureId, int mipMapLevel, X_Texture* dest);

void x_bsplevel_mark_surfaces_light_is_close_to(BspLevel* level, const X_Light* light, int currentFrame);

struct BoundSphere;
int x_bsplevel_find_nodes_intersecting_sphere(BspLevel* level, struct BoundSphere* sphere, X_BspNode** dest);

void x_bsplevel_render_submodels(BspLevel* level, struct X_RenderContext* renderContext);

//======================== level ========================

static inline bool x_bsplevel_file_is_loaded(const BspLevel* level)
{
    return (level->flags & X_BSPLEVEL_LOADED) != 0;
}

static inline X_BspLeaf* x_bsplevel_get_leaf(const BspLevel* level, X_BspLeafId leafId)
{
    return level->leaves + leafId;
}

static inline BspModel* x_bsplevel_get_level_model(const BspLevel* level)
{
    return level->models + 0;
}

static inline X_BspNode* x_bsplevel_get_root_node(const BspLevel* level)
{
    return x_bsplevel_get_level_model(level)->rootBspNode;
}

static inline void x_bsplevel_reset_bspkeys(BspLevel* level)
{
    level->nextBspKey = 1;
}

static inline void x_bsplevel_next_bspkey(BspLevel* level)
{
    ++level->nextBspKey;
}

static inline int x_bsplevel_current_bspkey(const BspLevel* level)
{
    return level->nextBspKey;
}

static inline BspModel* x_bsplevel_get_model(const BspLevel* level, int modelId)
{
    return level->models + modelId;
}

//======================== surface ========================

static inline bool x_bspsurface_is_visible_this_frame(const BspSurface* surface, int currentFrame)
{
    return surface->lastVisibleFrame == currentFrame;
}

static inline bool x_bspsurface_plane_is_flipped(const BspSurface* surface)
{
    return surface->flags & X_BSPSURFACE_FLIPPED;
}

//======================== pvs ========================

static inline int x_bspfile_node_pvs_size(const BspLevel* level)
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

static inline void x_bspboundrect_add_point(X_BspBoundRect* rect, Vec2 point)
{
    rect->v[0].x = X_MIN(rect->v[0].x, point.x);
    rect->v[0].y = X_MIN(rect->v[0].y, point.y);
    
    rect->v[1].x = X_MAX(rect->v[1].x, point.x);
    rect->v[1].y = X_MAX(rect->v[1].y, point.y);
}

//======================== model ========================

static inline x_fp16x16 x_bspmodel_height(BspModel* model)
{
    return model->boundBox.v[0].y - model->boundBox.v[1].y;
}

static inline bool x_bspmodel_has_objects_standing_on(BspModel* model)
{
    return model->objectsOnModelHead.next != &model->objectsOnModelTail;
}

