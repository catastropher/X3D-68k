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

static X_Vec3_float x_bsplevelloader_convert_coordinate(const X_Vec3_float* v)
{
    return x_vec3_float_make(v->y, -v->z, -v->x);
}

static void x_bsploaderlump_read_from_file(X_BspLoaderLump* lump, X_File* file)
{
    lump->fileOffset = x_file_read_le_int32(file);
    lump->length = x_file_read_le_int32(file);
}

static void x_bsploaderheader_read_from_file(X_BspLoaderHeader* header, X_File* file)
{
    header->bspVersion = x_file_read_le_int32(file);
    
    for(int i = 0; i < X_BSP_TOTAL_LUMPS; ++i)
        x_bsploaderlump_read_from_file(header->lumps + i, file);
}

static void x_bsploaderplane_read_from_file(X_BspLoaderPlane* plane, X_File* file)
{
    X_Vec3_float normal;
    x_file_read_vec3_float(file, &normal);
    normal = x_bsplevelloader_convert_coordinate(&normal);
    
    plane->plane.normal = x_vec3_float_to_vec3_fp16x16(&normal);
    
    float dist = x_file_read_le_float32(file);
    plane->plane.d = -x_fp16x16_from_float(dist);
    
    plane->type = x_file_read_le_int32(file);
}

static void x_bsploadervertex_read_from_file(X_BspLoaderVertex* vertex, X_File* file)
{
    X_Vec3_float v;
    x_file_read_vec3_float(file, &v);
    
    v = x_bsplevelloader_convert_coordinate(&v);
    vertex->v = x_vec3_float_to_vec3(&v);
}

static void x_bsploaderface_read_from_file(X_BspLoaderFace* face, X_File* file)
{
    face->planeNum = x_file_read_le_int16(file);
    face->side = x_file_read_le_int16(file);
    face->firstEdge = x_file_read_le_int32(file);
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
    leaf->numMarkSurface = x_file_read_le_int16(file);
    
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

static void x_bsploaderlevel_load_planes(X_BspLevelLoader* level, X_File* file)
{
    const int PLANE_SIZE_IN_FILE = 20;
    X_BspLoaderLump* planeLump = level->header.lumps + X_LUMP_PLANES;
    
    level->totalPlanes = planeLump->length / PLANE_SIZE_IN_FILE;
    level->planes = malloc(level->totalPlanes * sizeof(X_BspLoaderPlane));
    
    x_log("Total planes: %d", level->totalPlanes);
    
    x_file_seek(file, planeLump->fileOffset);
    
    for(int i = 0; i < level->totalPlanes; ++i)
    {
        x_bsploaderplane_read_from_file(level->planes + i, file);
    }
}

static void x_bsploaderlevel_load_vertices(X_BspLevelLoader* level, X_File* file)
{
    const int VERTEX_SIZE_IN_FILE = 12;
    X_BspLoaderLump* vertexLump = level->header.lumps + X_LUMP_VERTEXES;
    
    level->totalVertices = vertexLump->length / VERTEX_SIZE_IN_FILE;
    level->vertices = x_malloc(level->totalVertices * sizeof(X_BspLoaderVertex));
    
    x_log("Total vertices: %d", level->totalVertices);
    
    x_file_seek(file, vertexLump->fileOffset);
    
    for(int i = 0; i < level->totalVertices; ++i)
    {
        x_bsploadervertex_read_from_file(level->vertices + i, file);
    }
}

static void x_bsploaderlevel_load_faces(X_BspLevelLoader* level, X_File* file)
{
    const int FACE_SIZE_IN_FILE = 20;
    X_BspLoaderLump* faceLump = level->header.lumps + X_LUMP_FACES;
    
    level->totalFaces = faceLump->length / FACE_SIZE_IN_FILE;
    level->faces = x_malloc(level->totalFaces * sizeof(X_BspLoaderFace));
    
    x_log("Total faces: %d", level->totalFaces);
    
    x_file_seek(file, faceLump->fileOffset);
    
    for(int i = 0; i < level->totalFaces; ++i)
    {
        x_bsploaderface_read_from_file(level->faces + i, file);
    }
}

static void x_bsploaderlevel_load_leaves(X_BspLevelLoader* level, X_File* file)
{
    const int LEAF_SIZE_IN_FILE = 28;
    X_BspLoaderLump* leafLump = level->header.lumps + X_LUMP_LEAFS;
    
    level->totalLeaves = leafLump->length / LEAF_SIZE_IN_FILE;
    level->leaves = x_malloc(level->totalLeaves * sizeof(X_BspLoaderLeaf));
    
    x_log("Total leaves: %d", level->totalLeaves);
    
    x_file_seek(file, leafLump->fileOffset);
    
    for(int i = 0; i < level->totalLeaves; ++i)
    {
        x_bsploaderleaf_read_from_file(level->leaves + i, file);
    }
}

static void x_bsploaderlevel_load_nodes(X_BspLevelLoader* level, X_File* file)
{
    const int NODE_SIZE_IN_FILE = 24;
    X_BspLoaderLump* nodeLump = level->header.lumps + X_LUMP_NODES;
    
    level->totalNodes = nodeLump->length / NODE_SIZE_IN_FILE;
    level->nodes = x_malloc(level->totalNodes * sizeof(X_BspLoaderNode));
    
    x_log("Total nodes: %d\n", level->totalNodes);
    
    x_file_seek(file, nodeLump->fileOffset);
    
    for(int i = 0; i < level->totalNodes; ++i)
    {
        x_bsploadernode_read_from_file(level->nodes + i, file);
    }
}

static void x_bsploaderlevel_load_edges(X_BspLevelLoader* level, X_File* file)
{
    const int EDGE_SIZE_IN_FILE = 4;
    X_BspLoaderLump* edgeLump = level->header.lumps + X_LUMP_EDGES;
    
    level->totalEdges = edgeLump->length / EDGE_SIZE_IN_FILE;
    level->edges = x_malloc(level->totalEdges * sizeof(X_BspLoaderVertex));
    
    x_log("Total edges: %d", level->totalEdges);
    
    x_file_seek(file, edgeLump->fileOffset);
    
    for(int i = 0; i < level->totalEdges; ++i)
    {
        x_bsploaderedge_read_from_file(level->edges + i, file);
    }
}

static void x_bsploaderlevel_load_models(X_BspLevelLoader* level, X_File* file)
{
    const int MODEL_SIZE_IN_FILE = 64;
    X_BspLoaderLump* modelLump = level->header.lumps + X_LUMP_MODELS;
    
    level->totalModels = modelLump->length / MODEL_SIZE_IN_FILE;
    level->models = x_malloc(level->totalEdges * sizeof(X_BspLoaderModel));
    
    x_log("Total models: %d", level->totalModels);
    
    x_file_seek(file, modelLump->fileOffset);
    
    for(int i = 0; i < level->totalModels; ++i)
    {
        x_bsploadermodel_read_from_file(level->models + i, file);
    }
}

static void x_bsploaderlevel_load_compressed_pvs(X_BspLevelLoader* level)
{
    X_BspLoaderLump* pvsLump = level->header.lumps + X_LUMP_VISIBILITY;
    
    level->compressedPvsData = x_malloc(pvsLump->length);
    x_file_seek(&level->file, pvsLump->fileOffset);
    x_file_read_buf(&level->file, pvsLump->length, level->compressedPvsData);
}

_Bool x_bsplevel_load_from_bsp_file(X_BspLevel* level, const char* fileName)
{
    X_BspLevelLoader loader;
    
    if(!x_file_open_reading(&loader.file, fileName))
    {
        x_log_error("Failed to open BSP file %s\n", fileName);
        return 0;
    }
    
    x_bsploaderheader_read_from_file(&loader.header, &loader.file);
    x_log("BSP version: %d", loader.header.bspVersion);
    
    if(loader.header.bspVersion != 29)
        return 0;
    
    x_bsploaderlevel_load_compressed_pvs(&loader);
    x_bsploaderlevel_load_planes(&loader, &loader.file);
    x_bsploaderlevel_load_vertices(&loader, &loader.file);
    x_bsploaderlevel_load_edges(&loader, &loader.file);
    x_bsploaderlevel_load_faces(&loader, &loader.file);
    x_bsploaderlevel_load_leaves(&loader, &loader.file);
    x_bsploaderlevel_load_nodes(&loader, &loader.file);
    x_bsploaderlevel_load_models(&loader, &loader.file);
    
    level->flags = X_BSPLEVEL_LOADED;
    
    return 1;
}
