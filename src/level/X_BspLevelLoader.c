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

#include "X_BspLevel.h"
#include "X_BspLevelLoader.h"
#include "system/X_File.h"
#include "error/X_log.h"
#include "memory/X_alloc.h"
#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"
#include "util/X_util.h"
#include "error/X_error.h"

static X_Vec3_float x_bsplevelloader_convert_coordinate_float(const X_Vec3_float* v)
{
    return x_vec3_float_make(v->y, -v->z, -v->x);
}

static X_Vec3 x_bsplevelloader_convert_coordinate(const X_Vec3* v)
{
    return x_vec3_make(v->y, -v->z, -v->x);
}

static void x_bspboundbox_convert_coordinate(X_BspBoundBox* box)
{
    box->v[0] = x_bsplevelloader_convert_coordinate(box->v + 0);
    box->v[1] = x_bsplevelloader_convert_coordinate(box->v + 1);
    
    X_BspBoundBox temp;
    
    temp.v[0].x = X_MIN(box->v[0].x, box->v[1].x);
    temp.v[0].y = X_MIN(box->v[0].y, box->v[1].y);
    temp.v[0].z = X_MIN(box->v[0].z, box->v[1].z);
    
    temp.v[1].x = X_MAX(box->v[0].x, box->v[1].x);
    temp.v[1].y = X_MAX(box->v[0].y, box->v[1].y);
    temp.v[1].z = X_MAX(box->v[0].z, box->v[1].z);
    
    *box = temp;
}

static void x_bsploaderlump_read_from_file(X_BspLoaderLump* lump, X_File* file)
{
    lump->fileOffset = x_file_read_le_int32(file);
    lump->length = x_file_read_le_int32(file);
}

static void x_bsploadermiptexturelump_read_from_file(X_BspLoaderMipTextureLump* lump, X_File* file)
{
    lump->totalMipTextures = x_file_read_le_int32(file);
    lump->mipTextureOffsets = x_malloc(lump->totalMipTextures * sizeof(int));
    
    for(int i = 0; i < lump->totalMipTextures; ++i)
        lump->mipTextureOffsets[i] = x_file_read_le_int32(file);
}

static void x_bsploadermiptexturelump_cleanup(X_BspLoaderMipTextureLump* lump)
{
    x_free(lump->mipTextureOffsets);
}

static void x_bsploaderheader_read_from_file(X_BspLoaderHeader* header, X_File* file)
{
    header->bspVersion = x_file_read_le_int32(file);
    
    for(int i = 0; i < X_BSP_TOTAL_LUMPS; ++i)
        x_bsploaderlump_read_from_file(header->lumps + i, file);
}

static void x_bsptexture_read_from_file(X_BspTexture* texture, X_File* file)
{
    x_file_read_buf(file, 16, texture->name);
    texture->w = x_file_read_le_int32(file);
    texture->h = x_file_read_le_int32(file);
    
    for(int i = 0; i < 4; ++i)
        texture->texelsOffset[i] = x_file_read_le_int32(file);
}

//static void x_bspfacetexture_read_from_file(X_BspFaceTexture)

static void x_bsploaderplane_read_from_file(X_BspLoaderPlane* plane, X_File* file)
{
    X_Vec3_float normal;
    x_file_read_vec3_float(file, &normal);
    normal = x_bsplevelloader_convert_coordinate_float(&normal);
    
    plane->plane.normal = x_vec3_float_to_vec3_fp16x16(&normal);
    
    float dist = x_file_read_le_float32(file);
    plane->plane.d = -x_fp16x16_from_float(dist);
    
    plane->type = x_file_read_le_int32(file);
}

static void x_bsploadervertex_read_from_file(X_BspLoaderVertex* vertex, X_File* file)
{
    X_Vec3_float v;
    x_file_read_vec3_float(file, &v);
    
    v = x_bsplevelloader_convert_coordinate_float(&v);
    vertex->v = x_vec3_float_to_vec3(&v);
}

static void x_bsploaderface_read_from_file(X_BspLoaderFace* face, X_File* file)
{
    face->planeNum = x_file_read_le_int16(file);
    face->side = x_file_read_le_int16(file);
    face->firstEdge = x_file_read_le_int32(file);
    face->totalEdges = x_file_read_le_int16(file);
    face->texInfo = x_file_read_le_int16(file);
    
    for(int i = 0; i < X_BSPFACE_MAX_LIGHTMAPS; ++i)
        face->lightmapStypes[i] = x_file_read_char(file);
    
    face->lightmapOffset = x_file_read_le_int32(file);
}

static void x_bsploaderleaf_read_from_file(X_BspLoaderLeaf* leaf, X_File* file)
{
    leaf->contents = x_file_read_le_int32(file);    
    leaf->pvsOffset = x_file_read_le_int32(file);
    
    for(int i = 0; i < 3; ++i)
        leaf->mins[i] = x_file_read_le_int16(file);
    
    for(int i = 0; i < 3; ++i)
        leaf->maxs[i] = x_file_read_le_int16(file);
    
    leaf->firstMarkSurface = x_file_read_le_int16(file);
    leaf->totalMarkSurfaces = x_file_read_le_int16(file);
    
    for(int i = 0; i < X_BSPLEAF_TOTAL_AMBIENTS; ++i)
        leaf->ambientLevel[i] = x_file_read_char(file);
}

static void x_bsploadernode_read_from_file(X_BspLoaderNode* node, X_File* file)
{
    node->planeNum = x_file_read_le_int32(file);
    
    for(int i = 0; i < 2; ++i)
        node->children[i] = x_file_read_le_int16(file);
    
    for(int i = 0; i < 3; ++i)
        node->mins[i] = x_file_read_le_int16(file);
    
    for(int i = 0; i < 3; ++i)
        node->maxs[i] = x_file_read_le_int16(file);
    
    node->firstFace = x_file_read_le_int16(file);
    node->totalFaces = x_file_read_le_int16(file);
}

static void x_bsploaderedge_read_from_file(X_BspLoaderEdge* edge, X_File* file)
{
    edge->v[0] = x_file_read_le_int16(file);
    edge->v[1] = x_file_read_le_int16(file);
}

static void x_bsploadermodel_read_from_file(X_BspLoaderModel* model, X_File* file)
{
    for(int i = 0; i < 3; ++i)
        model->mins[i] = x_file_read_le_float32(file);
    
    for(int i = 0; i < 3; ++i)
        model->maxs[i] = x_file_read_le_float32(file);
    
    for(int i = 0; i < 3; ++i)
        model->origin[i] = x_file_read_le_float32(file);
    
    model->rootBspNode = x_file_read_le_int32(file);
    model->rootClipNode = x_file_read_le_int32(file);
    model->secondRootClipNode = x_file_read_le_int32(file);
    
    // Skip the unused field
    x_file_read_le_int32(file);
    
    model->totalBspLeaves = x_file_read_le_int32(file);
    model->firstFaceId = x_file_read_le_int32(file);
    model->totalFaces = x_file_read_le_int32(file);
}

static void x_bsplevelloader_load_planes(X_BspLevelLoader* level)
{
    const int PLANE_SIZE_IN_FILE = 20;
    X_BspLoaderLump* planeLump = level->header.lumps + X_LUMP_PLANES;
    
    level->totalPlanes = planeLump->length / PLANE_SIZE_IN_FILE;
    level->planes = x_malloc(level->totalPlanes * sizeof(X_BspLoaderPlane));
    
    x_log("Total planes: %d", level->totalPlanes);
    
    x_file_seek(&level->file, planeLump->fileOffset);
    
    for(int i = 0; i < level->totalPlanes; ++i)
        x_bsploaderplane_read_from_file(level->planes + i, &level->file);
}

static void x_bsplevelloader_load_vertices(X_BspLevelLoader* level)
{
    const int VERTEX_SIZE_IN_FILE = 12;
    X_BspLoaderLump* vertexLump = level->header.lumps + X_LUMP_VERTEXES;
    
    level->totalVertices = vertexLump->length / VERTEX_SIZE_IN_FILE;
    level->vertices = x_malloc(level->totalVertices * sizeof(X_BspLoaderVertex));
    
    x_log("Total vertices: %d", level->totalVertices);
    
    x_file_seek(&level->file, vertexLump->fileOffset);
    
    for(int i = 0; i < level->totalVertices; ++i)
        x_bsploadervertex_read_from_file(level->vertices + i, &level->file);
}

static void x_bsplevelloader_load_faces(X_BspLevelLoader* level)
{
    const int FACE_SIZE_IN_FILE = 20;
    X_BspLoaderLump* faceLump = level->header.lumps + X_LUMP_FACES;
    
    level->totalFaces = faceLump->length / FACE_SIZE_IN_FILE;
    level->faces = x_malloc(level->totalFaces * sizeof(X_BspLoaderFace));
    
    x_log("Total faces: %d", level->totalFaces);
    
    x_file_seek(&level->file, faceLump->fileOffset);
    
    for(int i = 0; i < level->totalFaces; ++i)
        x_bsploaderface_read_from_file(level->faces + i, &level->file);
}

static void x_bsplevelloader_load_leaves(X_BspLevelLoader* level)
{
    const int LEAF_SIZE_IN_FILE = 28;
    X_BspLoaderLump* leafLump = level->header.lumps + X_LUMP_LEAFS;
    
    level->totalLeaves = leafLump->length / LEAF_SIZE_IN_FILE;
    level->leaves = x_malloc(level->totalLeaves * sizeof(X_BspLoaderLeaf));
    
    x_log("Total leaves: %d", level->totalLeaves);
    
    x_file_seek(&level->file, leafLump->fileOffset);
    
    for(int i = 0; i < level->totalLeaves; ++i)
        x_bsploaderleaf_read_from_file(level->leaves + i, &level->file);
}

static void x_bsplevelloader_load_nodes(X_BspLevelLoader* level)
{
    const int NODE_SIZE_IN_FILE = 24;
    X_BspLoaderLump* nodeLump = level->header.lumps + X_LUMP_NODES;
    
    level->totalNodes = nodeLump->length / NODE_SIZE_IN_FILE;
    level->nodes = x_malloc(level->totalNodes * sizeof(X_BspLoaderNode));
    
    x_log("Total nodes: %d\n", level->totalNodes);
    
    x_file_seek(&level->file, nodeLump->fileOffset);
    
    for(int i = 0; i < level->totalNodes; ++i)
        x_bsploadernode_read_from_file(level->nodes + i, &level->file);
}

static void x_bsplevelloader_load_edges(X_BspLevelLoader* level)
{
    const int EDGE_SIZE_IN_FILE = 4;
    X_BspLoaderLump* edgeLump = level->header.lumps + X_LUMP_EDGES;
    
    level->totalEdges = edgeLump->length / EDGE_SIZE_IN_FILE;
    level->edges = x_malloc(level->totalEdges * sizeof(X_BspLoaderVertex));
    
    x_log("Total edges: %d", level->totalEdges);
    
    x_file_seek(&level->file, edgeLump->fileOffset);
    
    for(int i = 0; i < level->totalEdges; ++i)
        x_bsploaderedge_read_from_file(level->edges + i, &level->file);
}

static void x_bsplevelloader_load_models(X_BspLevelLoader* level)
{
    const int MODEL_SIZE_IN_FILE = 64;
    X_BspLoaderLump* modelLump = level->header.lumps + X_LUMP_MODELS;
    
    level->totalModels = modelLump->length / MODEL_SIZE_IN_FILE;
    level->models = x_malloc(level->totalEdges * sizeof(X_BspLoaderModel));
    
    x_log("Total models: %d", level->totalModels);
    
    x_file_seek(&level->file, modelLump->fileOffset);
    
    for(int i = 0; i < level->totalModels; ++i)
        x_bsploadermodel_read_from_file(level->models + i, &level->file);
}

static void x_bsplevelloader_load_compressed_pvs(X_BspLevelLoader* level)
{
    X_BspLoaderLump* pvsLump = level->header.lumps + X_LUMP_VISIBILITY;
    
    level->compressedPvsData = x_malloc(pvsLump->length);
    x_file_seek(&level->file, pvsLump->fileOffset);
    x_file_read_buf(&level->file, pvsLump->length, level->compressedPvsData);
}

static void x_bsplevelloader_load_marksurfaces(X_BspLevelLoader* loader)
{
    const int MARKSURFACE_SIZE_IN_FILE = 2;
    X_BspLoaderLump* markSurfaceLump = loader->header.lumps + X_LUMP_MARKSURFACES;
    
    loader->totalMarkSurfaces = markSurfaceLump->length / MARKSURFACE_SIZE_IN_FILE;
    loader->markSurfaces = x_malloc(sizeof(unsigned short) * loader->totalMarkSurfaces);
    
    x_log("Total mark surfaces: %d", loader->totalMarkSurfaces);
    
    x_file_seek(&loader->file, markSurfaceLump->fileOffset);
    
    for(int i = 0; i < loader->totalMarkSurfaces; ++i)
        loader->markSurfaces[i] = x_file_read_le_int16(&loader->file);
}

static void x_bsplevelloader_load_surfaceedgeids(X_BspLevelLoader* loader)
{
    const int SURFACEEDGEID_SIZE_IN_FILE = 4;
    X_BspLoaderLump* surfaceEdgeIdsLump = loader->header.lumps + X_LUMP_SURFEDGES;
    
    loader->totalSurfaceEdgeIds = surfaceEdgeIdsLump->length / SURFACEEDGEID_SIZE_IN_FILE;
    loader->surfaceEdgeIds = x_malloc(loader->totalSurfaceEdgeIds * sizeof(int));
    
    x_file_seek(&loader->file, surfaceEdgeIdsLump->fileOffset);
    
    x_log("Total surface edge ids: %d", loader->totalSurfaceEdgeIds);
    
    for(int i = 0; i < loader->totalSurfaceEdgeIds; ++i)
        loader->surfaceEdgeIds[i] = x_file_read_le_int32(&loader->file);
}

static void x_bsplevelloader_load_textures(X_BspLevelLoader* loader)
{
    const int TEXTURE_SIZE_IN_FILE = 40;
    X_BspLoaderLump* textureLump = loader->header.lumps + X_LUMP_TEXTURES;
    
    x_file_seek(&loader->file, textureLump->fileOffset);
    
    X_BspLoaderMipTextureLump mipLump;
    x_bsploadermiptexturelump_read_from_file(&mipLump, &loader->file);
    
    loader->totalTextures = mipLump.totalMipTextures;
    
    for(int i = 0; i < loader->totalTextures; ++i)
    {
        int textureFileOffset = textureLump->fileOffset + mipLump.mipTextureOffsets[i];
        x_file_seek(&loader->file, textureFileOffset);
        
        x_bsptexture_read_from_file(loader->textures + i, &loader->file);
    }
    
    x_bsploadermiptexturelump_cleanup(&mipLump);
}

static void x_bsplevel_allocate_memory(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    level->totalEdges = loader->totalEdges;
    level->edges = x_malloc(level->totalEdges * sizeof(X_BspEdge));
    
    level->totalSurfaces = loader->totalFaces;
    level->surfaces = x_malloc(level->totalSurfaces * sizeof(X_BspSurface));
    
    level->totalLeaves = loader->totalLeaves;
    level->leaves = x_malloc(level->totalLeaves * sizeof(X_BspLeaf));
    
    level->totalModels = loader->totalModels;
    level->models = x_malloc(level->totalModels * sizeof(X_BspModel));
    
    level->totalNodes = loader->totalNodes;
    level->nodes = x_malloc(level->totalNodes * sizeof(X_BspNode));
    
    level->totalVertices = loader->totalVertices;
    level->vertices = x_malloc(level->totalVertices * sizeof(X_BspVertex));
    
    level->totalPlanes = loader->totalPlanes;
    level->planes = x_malloc(level->totalPlanes * sizeof(X_BspPlane));
    
    level->totalMarkSurfaces = loader->totalMarkSurfaces;
    level->markSurfaces = x_malloc(level->totalMarkSurfaces * sizeof(X_BspSurface*));
}

static void x_bsplevel_init_vertices(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalVertices; ++i)
        level->vertices[i].v = loader->vertices[i].v;
}

static void x_bsplevel_init_surfaces(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < loader->totalFaces; ++i)
    {
        X_BspSurface* surface = level->surfaces + i;
        X_BspLoaderFace* face = loader->faces + i;
        
        surface->plane = level->planes + face->planeNum;
        surface->firstEdgeId = face->firstEdge;
        surface->totalEdges = face->totalEdges;
        surface->color = rand() % 256;
        surface->flags = 0;
        
        if(face->side != 0)
            surface->flags |= X_BSPSURFACE_FLIPPED;
    }
}

static void x_bsplevel_init_marksurfaces(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalMarkSurfaces; ++i)
        level->markSurfaces[i] = level->surfaces + loader->markSurfaces[i];
}

static void x_bsplevel_init_planes(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalPlanes; ++i)
        level->planes[i].plane = loader->planes[i].plane;
}

static void x_bsplevel_init_leaves(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalLeaves; ++i)
    {
        X_BspLeaf* leaf = level->leaves + i;
        X_BspLoaderLeaf* loadLeaf = loader->leaves + i;
        
        leaf->compressedPvsData = level->compressedPvsData + loadLeaf->pvsOffset;
        leaf->contents = loadLeaf->contents;
        leaf->lastVisibleFrame = 0;
        leaf->firstMarkSurface = level->markSurfaces + loadLeaf->firstMarkSurface;
        leaf->totalMarkSurfaces = loadLeaf->totalMarkSurfaces;
        
        leaf->boundBox.v[0].x = loadLeaf->mins[0];
        leaf->boundBox.v[0].y = loadLeaf->mins[1];
        leaf->boundBox.v[0].z = loadLeaf->mins[2];
        
        leaf->boundBox.v[1].x = loadLeaf->maxs[0];
        leaf->boundBox.v[1].y = loadLeaf->maxs[1];
        leaf->boundBox.v[1].z = loadLeaf->maxs[2];
        
        x_bspboundbox_convert_coordinate(&leaf->boundBox);
    }
}

static void x_bspnode_assign_parent(X_BspNode* node, X_BspNode* parent)
{
    node->parent = parent;
    
    if(x_bspnode_is_leaf(node))
        return;
    
    x_bspnode_assign_parent(node->frontChild, node);
    x_bspnode_assign_parent(node->backChild, node);
}

static X_BspNode* x_bsplevel_get_node_from_id(X_BspLevel* level, short id)
{
    if(id >= 0)
        return level->nodes + id;
    
    return (X_BspNode*)(level->leaves + (~id));
}

static void x_bsplevel_init_models(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalModels; ++i)
    {
        X_BspModel* model = level->models + i;
        X_BspLoaderModel* loadModel = loader->models + i;
        
        model->faces = level->surfaces + loadModel->firstFaceId;
        model->totalFaces = loadModel->totalFaces;
        
        model->rootBspNode = x_bsplevel_get_node_from_id(level, loadModel->rootBspNode);
        model->totalBspLeaves = loadModel->totalBspLeaves;
        
        x_bspnode_assign_parent(model->rootBspNode, NULL);
    }
}

static void x_bsplevel_init_nodes(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalNodes; ++i)
    {
        X_BspNode* node = level->nodes + i;
        X_BspLoaderNode* loadNode = loader->nodes + i;
        
        node->contents = X_BSPLEAF_NODE;
        node->frontChild = x_bsplevel_get_node_from_id(level, loadNode->children[0]);
        node->backChild = x_bsplevel_get_node_from_id(level, loadNode->children[1]);
        node->lastVisibleFrame = 0;
        node->plane = level->planes + loadNode->planeNum;
        node->firstSurface = level->surfaces + loadNode->firstFace;
        node->totalSurfaces = loadNode->totalFaces;
        
        node->boundBox.v[0].x = loadNode->mins[0];
        node->boundBox.v[0].y = loadNode->mins[1];
        node->boundBox.v[0].z = loadNode->mins[2];
        
        node->boundBox.v[1].x = loadNode->maxs[0];
        node->boundBox.v[1].y = loadNode->maxs[1];
        node->boundBox.v[1].z = loadNode->maxs[2];
        
        x_bspboundbox_convert_coordinate(&node->boundBox);
    }
}

static void x_bsplevel_init_edges(X_BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        level->edges[i].v[0] = loader->edges[i].v[0];
        level->edges[i].v[1] = loader->edges[i].v[1];
    }
}

static void x_bsplevel_init_pvs(X_BspLevel* level, X_BspLevelLoader* loader)
{
    // Steal the PVS (no sense in making a copy)
    level->compressedPvsData = loader->compressedPvsData;
    loader->compressedPvsData = NULL;
}

static void x_bsplevel_init_surfacedgeids(X_BspLevel* level, X_BspLevelLoader* loader)
{
    // Steal the list of surface edges
    level->surfaceEdgeIds = loader->surfaceEdgeIds;
    loader->surfaceEdgeIds = NULL;
}

static void x_bsplevel_init_from_bsplevel_loader(X_BspLevel* level, X_BspLevelLoader* loader)
{
    x_bsplevel_allocate_memory(level, loader);
    
    x_bsplevel_init_pvs(level, loader);
    x_bsplevel_init_vertices(level, loader);
    x_bsplevel_init_edges(level, loader);
    x_bsplevel_init_planes(level, loader);
    x_bsplevel_init_surfaces(level, loader);
    x_bsplevel_init_marksurfaces(level, loader);
    x_bsplevel_init_leaves(level, loader);
    x_bsplevel_init_nodes(level, loader);
    x_bsplevel_init_models(level, loader);
    x_bsplevel_init_surfacedgeids(level, loader);
}

_Bool x_bsplevelloader_load_bsp_file(X_BspLevelLoader* loader, const char* fileName)
{
    if(!x_file_open_reading(&loader->file, fileName))
    {
        x_log_error("Failed to open BSP file %s\n", fileName);
        return 0;
    }
    
    x_bsploaderheader_read_from_file(&loader->header, &loader->file);
    x_log("BSP version: %d", loader->header.bspVersion);
    
    if(loader->header.bspVersion != 29)
        return 0;
    
    x_bsplevelloader_load_compressed_pvs(loader);
    x_bsplevelloader_load_planes(loader);
    x_bsplevelloader_load_vertices(loader);
    x_bsplevelloader_load_edges(loader);
    x_bsplevelloader_load_faces(loader);
    x_bsplevelloader_load_leaves(loader);
    x_bsplevelloader_load_nodes(loader);
    x_bsplevelloader_load_models(loader);
    x_bsplevelloader_load_marksurfaces(loader);
    x_bsplevelloader_load_surfaceedgeids(loader);
    x_bsplevelloader_load_textures(loader);
    
    for(int i = 0; i < 1; ++i)
    {
        x_log("Texture name: %s\n", loader->textures[i].name);
    }
    
    return 1;
}

_Bool x_bsplevel_load_from_bsp_file(X_BspLevel* level, const char* fileName)
{
    X_BspLevelLoader loader;
    if(!x_bsplevelloader_load_bsp_file(&loader, fileName))
        return 0;
    
    x_bsplevel_init_from_bsplevel_loader(level, &loader);
    
    level->flags = X_BSPLEVEL_LOADED;
    
    return 1;
}

