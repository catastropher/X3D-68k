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

#include <math.h>
#include <new>

#include "BspLevel.hpp"
#include "BspLevelLoader.hpp"
#include "system/File.hpp"
#include "error/Log.hpp"
#include "memory/Alloc.h"
#include "render/RenderContext.hpp"
#include "geo/Ray3.hpp"
#include "util/Util.hpp"
#include "error/Error.hpp"
#include "BspLevelLoaderFileSizes.hpp"
#include "BspLevelLoaderStreamReader.hpp"

template<typename T>
void readLump(X_BspLoaderLump* lump, T* dest, int total, X_File& file)
{
    char* temp = (char*)x_malloc(lump->length);

    x_file_seek(&file, lump->fileOffset);
    x_file_read_buf(&file, lump->length, temp);

    StreamReader reader(temp, temp + lump->length);

    reader.readArray(dest, total);

    x_free(temp);
}

static void x_bsploadertexture_read_from_file(X_BspLoaderTexture* texture, X_File* file)
{
    x_file_read_buf(file, 16, texture->name);
    texture->w = x_file_read_le_int32(file);
    texture->h = x_file_read_le_int32(file);
    
    for(int mipTex = 0; mipTex < 4; ++mipTex)
        texture->texelsOffset[mipTex] = x_file_read_le_int32(file);
}

static void x_boundbox_convert_coordinate(BoundBox* box)
{
    box->v[0] = box->v[0].toX3dCoords();
    box->v[1] = box->v[1].toX3dCoords();
    
    BoundBox temp;
    
    temp.v[0].x = X_MIN(box->v[0].x, box->v[1].x);
    temp.v[0].y = X_MIN(box->v[0].y, box->v[1].y);
    temp.v[0].z = X_MIN(box->v[0].z, box->v[1].z);
    
    temp.v[1].x = X_MAX(box->v[0].x, box->v[1].x);
    temp.v[1].y = X_MAX(box->v[0].y, box->v[1].y);
    temp.v[1].z = X_MAX(box->v[0].z, box->v[1].z);
    
    for(int i = 0; i < 2; ++i)
        temp.v[i] = x_vec3_int_to_vec3(temp.v + i);
    
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
    lump->mipTextureOffsets = (int*)x_malloc(lump->totalMipTextures * sizeof(int));
    
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

template<>
StreamReader& StreamReader::read(X_BspLoaderTexture& texture)
{
    readArray(texture.name, 16)
        .read(texture.w)
        .read(texture.h)
        .readArray(texture.texelsOffset, 4);

    return *this;
}

const int totalSteps = 15;

template<typename T>
void X_BspLevelLoader::loadLump(int lumpId, Array<T>& dest, const char* name)
{
    X_BspLoaderLump& lump = header.lumps[lumpId];
    
    dest.count = lump.length / sizeInFile<T>();
    dest.elem = (T*)x_malloc(dest.count * sizeof(T));

    Log::logSub("Total %s: %d", name, dest.count);

    readLump(&lump, dest.elem, dest.count, file);

    ++progressEvent.levelLoadProgress.currentStep;
    sendProgressEvent();
}

static void x_bsplevelloader_load_entity_dictionary(X_BspLevelLoader* loader)
{
    X_BspLoaderLump* entityDictionaryLump = loader->header.lumps + X_LUMP_ENTITIES;
    
    loader->entityDictionary = (char*)x_malloc(entityDictionaryLump->length + 1);
    
    x_file_seek(&loader->file, entityDictionaryLump->fileOffset);
    x_file_read_buf(&loader->file, entityDictionaryLump->length, loader->entityDictionary);
    
    loader->entityDictionary[entityDictionaryLump->length] = '\0';
}

static int x_bsploadertexture_calculate_needed_texels_for_mipmaps(X_BspLoaderTexture* tex)
{
    // w * h + (w / 2) * (h / 2) + (w / 4) * (h / 4) + (w / 8) * (h / 8)
    return tex->w * tex->h / 64 * 85;
}

static void x_bsplevelloader_load_textures(X_BspLevelLoader* loader)
{
    X_BspLoaderLump* textureLump = loader->header.lumps + X_LUMP_TEXTURES;
    
    x_file_seek(&loader->file, textureLump->fileOffset);
    
    X_BspLoaderMipTextureLump mipLump;
    x_bsploadermiptexturelump_read_from_file(&mipLump, &loader->file);
    
    loader->totalTextures = mipLump.totalMipTextures;
    loader->textures = (X_BspLoaderTexture*)x_malloc(loader->totalTextures * sizeof(BspTexture));
    
    int totalTexels = 0;
    
    const int INVALID_TEXTURE_OFFSET = -1;
    
    for(int i = 0; i < loader->totalTextures; ++i)
    {
        if(mipLump.mipTextureOffsets[i] == INVALID_TEXTURE_OFFSET)
        {
            for(int j = 0; j < 4; ++j)
                loader->textures[i].texelsOffset[j] = INVALID_TEXTURE_OFFSET;
            
            continue;
        }
        
        int textureFileOffset = textureLump->fileOffset + mipLump.mipTextureOffsets[i];
        x_file_seek(&loader->file, textureFileOffset);
        x_bsploadertexture_read_from_file(loader->textures + i, &loader->file);
        
        totalTexels += x_bsploadertexture_calculate_needed_texels_for_mipmaps(loader->textures + i);
    }
    
    // Allocate the texels for all of the textures in one giant allocation
    loader->textureTexels = (X_Color*)x_malloc(totalTexels * sizeof(X_Color));
    
    X_Color* texels = loader->textureTexels;    
    for(int i = 0; i < loader->totalTextures; ++i)
    {
        if(mipLump.mipTextureOffsets[i] == INVALID_TEXTURE_OFFSET)
            continue;
        
        X_BspLoaderTexture* tex = loader->textures + i;
        int texelsInMipTexture = tex->w * tex->h;
        
        const int TEXTURE_SIZE_IN_FILE = 40;
        int texelsOffset = textureLump->fileOffset + mipLump.mipTextureOffsets[i] + TEXTURE_SIZE_IN_FILE;
        x_file_seek(&loader->file, texelsOffset);
        
        for(int mipTex = 0; mipTex < 4; ++mipTex)
        {
            x_file_read_buf(&loader->file, texelsInMipTexture, texels);
            tex->texelsOffset[mipTex] = texels - loader->textureTexels;
            texels += texelsInMipTexture;
            texelsInMipTexture /= 4;
        }
    }
    
    x_bsploadermiptexturelump_cleanup(&mipLump);
}

static void x_bsplevelloader_load_facetextures(X_BspLevelLoader* loader)
{
    //x_bsploaderfacetexture_read_from_file
    
    int FACE_TEXTURE_SIZE_IN_FILE = 40;
    X_BspLoaderLump* faceTextureLump = loader->header.lumps + X_LUMP_TEXINFO;
    
    loader->totalFaceTextures = faceTextureLump->length /  FACE_TEXTURE_SIZE_IN_FILE;
    loader->faceTextures = (X_BspLoaderFaceTexture*)x_malloc(loader->totalFaceTextures * sizeof(X_BspLoaderFaceTexture));
    
    x_log("Total face textures: %d", loader->totalFaceTextures);

    readLump(faceTextureLump, loader->faceTextures, loader->totalFaceTextures, loader->file);
}

static void x_bsplevelloader_init_collision_hulls(X_BspLevelLoader* loader)
{
    X_BspLoaderModel* levelModel = loader->models.elem + 0;
    
    loader->collisionHulls[0].rootNode = levelModel->rootBspNode;
    loader->collisionHulls[1].rootNode = levelModel->rootClipNode;
    loader->collisionHulls[2].rootNode = levelModel->secondRootClipNode;
    loader->collisionHulls[3].rootNode = levelModel->thirdRootClipNode;
}

static void x_bsplevel_allocate_memory(BspLevel* level, const X_BspLevelLoader* loader)
{
    level->totalEdges = loader->edges.count;
    level->edges = (BspEdge*)x_malloc(level->totalEdges * sizeof(BspEdge));
    
    level->totalSurfaces = loader->faces.count;
    level->surfaces = (BspSurface*)x_malloc(level->totalSurfaces * sizeof(BspSurface));
    
    level->totalLeaves = loader->leaves.count;
    level->leaves = (X_BspLeaf*)x_malloc(level->totalLeaves * sizeof(X_BspLeaf));
    
    level->totalModels = loader->models.count;
    level->models = (BspModel*)x_malloc(level->totalModels * sizeof(BspModel));
    
    level->totalNodes = loader->nodes.count;
    level->nodes = (X_BspNode*)x_malloc(level->totalNodes * sizeof(X_BspNode));
    
    level->totalVertices = loader->vertices.count;
    level->vertices = (BspVertex*)x_malloc(level->totalVertices * sizeof(BspVertex));
    
    level->totalPlanes = loader->planes.count;
    level->planes = (BspPlane*)x_malloc(level->totalPlanes * sizeof(BspPlane));
    
    level->totalMarkSurfaces = loader->markSurfaces.count;
    level->markSurfaces = (BspSurface**)x_malloc(level->totalMarkSurfaces * sizeof(BspSurface*));
    
    level->totalTextures = loader->totalTextures;
    level->textures = (BspTexture*)x_malloc(level->totalTextures * sizeof(BspTexture));
    
    level->totalFaceTextures = loader->totalFaceTextures;
    level->faceTextures = (BspFaceTexture*)x_malloc(level->totalFaceTextures * sizeof(BspFaceTexture));
}

static void x_bsplevel_init_vertices(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalVertices; ++i)
    {
        level->vertices[i].v = loader->vertices.elem[i].v;
    }
}

static Vec2 x_bspsurface_calculate_texture_coordinate_of_vertex(BspSurface* surface, Vec3fp& v)
{    
    return Vec2(
        surface->faceTexture->uOrientation.dot(v).toFp16x16() + surface->faceTexture->uOffset,
        surface->faceTexture->vOrientation.dot(v).toFp16x16() + surface->faceTexture->vOffset);
}

static void x_bspsurface_calculate_texture_extents(BspSurface* surface, BspLevel* level)
{
    X_BspBoundRect textureCoordsBoundRect;
    x_bspboundrect_init(&textureCoordsBoundRect);
    
    for(int i = 0; i < surface->totalEdges; ++i)
    {
        BspVertex* v;
        int edgeId = level->surfaceEdgeIds[surface->firstEdgeId + i];
        
        if(edgeId >= 0)
            v = level->vertices + level->edges[edgeId].v[1];
        else
            v = level->vertices + level->edges[-edgeId].v[0];
        
        Vec2 textureCoord = x_bspsurface_calculate_texture_coordinate_of_vertex(surface, v->v);
        x_bspboundrect_add_point(&textureCoordsBoundRect, textureCoord);
    }
    
    textureCoordsBoundRect.v[0].x = floor((float)textureCoordsBoundRect.v[0].x / (16 * 65536));
    textureCoordsBoundRect.v[0].y = floor((float)textureCoordsBoundRect.v[0].y / (16 * 65536));
    
    textureCoordsBoundRect.v[1].x = ceil((float)textureCoordsBoundRect.v[1].x / (16 * 65536));
    textureCoordsBoundRect.v[1].y = ceil((float)textureCoordsBoundRect.v[1].y / (16 * 65536));
    
    surface->textureMinCoord = Vec2(
        textureCoordsBoundRect.v[0].x * 16 * 65536,
        textureCoordsBoundRect.v[0].y * 16 * 65536);
        
    surface->textureExtent = Vec2(
        (textureCoordsBoundRect.v[1].x - textureCoordsBoundRect.v[0].x) * 16 * 65536,
        (textureCoordsBoundRect.v[1].y - textureCoordsBoundRect.v[0].y) * 16 * 65536);
}

static void x_bsplevel_init_surfaces(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < loader->faces.count; ++i)
    {
        BspSurface* surface = level->surfaces + i;
        X_BspLoaderFace* face = loader->faces.elem + i;
        
        surface->id = i;
        surface->plane = level->planes + face->planeNum;
        surface->firstEdgeId = face->firstEdge;
        surface->totalEdges = face->totalEdges;
        surface->color = rand() % 256;
        surface->flags = (X_BspSurfaceFlags)0;
        
        if(face->side != 0)
            surface->flags = (X_BspSurfaceFlags)(surface->flags | X_BSPSURFACE_FLIPPED);
        
        surface->faceTexture = level->faceTextures + face->texInfo;
        surface->lightmapData = level->lightmapData + face->lightmapOffset;
        surface->lastVisibleFrame = -1;
        
        x_bspsurface_calculate_texture_extents(surface, level);
        
        for(int j = 0; j < 4; ++j)
        {
            x_cacheentry_init(surface->cachedSurfaces + j);
            surface->lightmapStyles[j] = face->lightmapStyles[j];
        }
    }
}

static void x_bsplevel_init_marksurfaces(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalMarkSurfaces; ++i)
        level->markSurfaces[i] = level->surfaces + loader->markSurfaces.elem[i];
}

static void x_bsplevel_init_planes(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalPlanes; ++i)
    {
        level->planes[i].plane.normal.x = loader->planes.elem[i].normal.x.toFp16x16();
        level->planes[i].plane.normal.y = loader->planes.elem[i].normal.y.toFp16x16();
        level->planes[i].plane.normal.z = loader->planes.elem[i].normal.z.toFp16x16();

        level->planes[i].plane.d = -loader->planes.elem[i].d.toFp16x16();
    }
}

static void x_bsplevel_init_leaves(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalLeaves; ++i)
    {
        X_BspLeaf* leaf = level->leaves + i;
        X_BspLoaderLeaf* loadLeaf = loader->leaves.elem + i;
        
        leaf->pvsFromLeaf.setCompressedBytes(level->pvs.getCompressedPvsData() + loadLeaf->pvsOffset);
        leaf->contents = (X_BspLeafContents)loadLeaf->contents;
        leaf->lastVisibleFrame = 0;
        leaf->firstMarkSurface = level->markSurfaces + loadLeaf->firstMarkSurface;
        leaf->totalMarkSurfaces = loadLeaf->totalMarkSurfaces;
        
        leaf->nodeBoundBox.v[0].x = loadLeaf->mins[0];
        leaf->nodeBoundBox.v[0].y = loadLeaf->mins[1];
        leaf->nodeBoundBox.v[0].z = loadLeaf->mins[2];
        
        leaf->nodeBoundBox.v[1].x = loadLeaf->maxs[0];
        leaf->nodeBoundBox.v[1].y = loadLeaf->maxs[1];
        leaf->nodeBoundBox.v[1].z = loadLeaf->maxs[2];
        
        x_boundbox_convert_coordinate(&leaf->nodeBoundBox);
    }
}

static void x_bspnode_assign_parent(X_BspNode* node, X_BspNode* parent)
{
    node->parent = parent;
    
    if(node->isLeaf())
        return;
    
    x_bspnode_assign_parent(node->frontChild, node);
    x_bspnode_assign_parent(node->backChild, node);
}

static X_BspNode* x_bsplevel_get_node_from_id(BspLevel* level, short id)
{
    if(id >= 0)
        return level->nodes + id;
    
    return (X_BspNode*)(level->leaves + (~id));
}

static void x_bsplevel_init_models(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalModels; ++i)
    {
        BspModel* model = level->models + i;
        X_BspLoaderModel* loadModel = loader->models.elem + i;
        
        model->faces = level->surfaces + loadModel->firstFaceId;
        model->totalFaces = loadModel->totalFaces;
        model->surfaceEdgeIds = level->surfaceEdgeIds;
        
        model->rootBspNode = x_bsplevel_get_node_from_id(level, loadModel->rootBspNode);
        model->totalBspLeaves = loadModel->totalBspLeaves;

        model->planes = level->planes;
        
        model->clipNodes = level->clipNodes;
        model->clipNodeRoots[0] = loadModel->rootClipNode;
        model->clipNodeRoots[1] = loadModel->secondRootClipNode;
        model->clipNodeRoots[2] = loadModel->thirdRootClipNode;
        
        model->center = convert<Vec3fp>(loadModel->origin)
            .toX3dCoords();
        
        model->boundBox.v[0].x = x_fp16x16_from_float(loadModel->mins[0]);
        model->boundBox.v[0].y = x_fp16x16_from_float(loadModel->mins[1]);
        model->boundBox.v[0].z = x_fp16x16_from_float(loadModel->mins[2]);
        
        model->boundBox.v[1].x = x_fp16x16_from_float(loadModel->maxs[0]);
        model->boundBox.v[1].y = x_fp16x16_from_float(loadModel->maxs[1]);
        model->boundBox.v[1].z = x_fp16x16_from_float(loadModel->maxs[2]);
        
        model->edges = level->edges;
        model->vertices = level->vertices;
        model->flags = 0;
        
        x_link_init(&model->objectsOnModelHead, &model->objectsOnModelTail);
        
        for(int i = 0; i < 2; ++i)
            model->boundBox.v[i] = x_vec3_convert_quake_coord_to_x3d_coord(model->boundBox.v + i);

        // The extents get swapped because of the coordinate shift
        std::swap(model->boundBox.v[0].y, model->boundBox.v[1].y);
        std::swap(model->boundBox.v[0].z, model->boundBox.v[1].z);
        
        x_bspnode_assign_parent(model->rootBspNode, NULL);
    }
}

static void x_bsplevel_init_nodes(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalNodes; ++i)
    {
        X_BspNode* node = level->nodes + i;
        X_BspLoaderNode* loadNode = loader->nodes.elem + i;
        
        node->contents = X_BSPLEAF_NODE;
        node->frontChild = x_bsplevel_get_node_from_id(level, loadNode->children[0]);
        node->backChild = x_bsplevel_get_node_from_id(level, loadNode->children[1]);
        node->lastVisibleFrame = 0;
        node->plane = level->planes + loadNode->planeNum;
        node->firstSurface = level->surfaces + loadNode->firstFace;
        node->totalSurfaces = loadNode->totalFaces;
        
        node->nodeBoundBox.v[0].x = loadNode->mins[0];
        node->nodeBoundBox.v[0].y = loadNode->mins[1];
        node->nodeBoundBox.v[0].z = loadNode->mins[2];
        
        node->nodeBoundBox.v[1].x = loadNode->maxs[0];
        node->nodeBoundBox.v[1].y = loadNode->maxs[1];
        node->nodeBoundBox.v[1].z = loadNode->maxs[2];
        
        x_boundbox_convert_coordinate(&node->nodeBoundBox);
    }
}

static void x_bsplevel_init_edges(BspLevel* level, const X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        level->edges[i].v[0] = loader->edges.elem[i].v[0];
        level->edges[i].v[1] = loader->edges.elem[i].v[1];
        level->edges[i].cachedEdgeOffset = 0;
    }
}

static void x_bsplevel_init_pvs(BspLevel* level, X_BspLevelLoader* loader)
{
    // Steal the PVS (no sense in making a copy)
    level->pvs.setCompressedPvsData(loader->compressedPvsData.elem);
    loader->compressedPvsData.elem = NULL;
}

static void x_bsplevel_init_surfacedgeids(BspLevel* level, X_BspLevelLoader* loader)
{
    // Steal the list of surface edges
    level->surfaceEdgeIds = loader->surfaceEdgeIds.elem;
    loader->surfaceEdgeIds.elem = NULL;
}

static void x_bsplevel_init_textures(BspLevel* level, X_BspLevelLoader* loader)
{
    // Steal the loaded texels
    level->textureTexels = loader->textureTexels;
    loader->textureTexels = NULL;
    
    for(int i = 0; i < level->totalTextures; ++i)
    {
        BspTexture* tex = level->textures + i;
        X_BspLoaderTexture* loadTex = loader->textures + i;
    
        const int INVALID_TEXTURE_OFFSET = -1;
        if((int)loadTex->texelsOffset[0] == INVALID_TEXTURE_OFFSET)
        {
            for(int j = 0; j < 4; ++j)
                tex->mipTexels[j] = NULL;
            
            continue;
        }
        
        strcpy(tex->name, loadTex->name);
        tex->w = loadTex->w;
        tex->h = loadTex->h;
        
        for(int mipTex = 0; mipTex < 4; ++mipTex)
        {
            tex->mipTexels[mipTex] = level->textureTexels + loadTex->texelsOffset[mipTex];
        }
    }
}

static void x_bsplevel_init_facetextures(BspLevel* level, X_BspLevelLoader* loader)
{
    for(int i = 0; i < level->totalFaceTextures; ++i)
    {
        BspFaceTexture* tex = level->faceTextures + i;
        X_BspLoaderFaceTexture* loadTex = loader->faceTextures + i;
        
        tex->uOrientation = loadTex->uOrientation;
        tex->uOffset = loadTex->uOffset.toFp16x16();
        
        tex->vOrientation = loadTex->vOrientation;
        tex->vOffset = loadTex->vOffset.toFp16x16();
        
        tex->flags = loadTex->flags;
        tex->texture = level->textures + loadTex->textureId;
    }
}

static void x_bspnode_calculate_geo_boundbox_add_surface(X_BspNode* node, BspSurface* surface, BspLevel* level)
{
    for(int i = 0; i < surface->totalEdges; ++i)
    {
        int edgeId = level->surfaceEdgeIds[surface->firstEdgeId + i];
        Vec3fp v;
        
        if(edgeId > 0)
            v = level->vertices[level->edges[edgeId].v[1]].v;
        else
            v = level->vertices[level->edges[-edgeId].v[0]].v;
        
        Vec3Template<int> vInt(v.x.toInt(), v.y.toInt(), v.z.toInt());
        
        node->geoBoundBox.addPoint(vInt);
    }
}

static void x_bspnode_calculate_geo_boundbox(X_BspNode* node, BspLevel* level)
{
    if(node->isLeaf())
        return;
    
    new (&node->geoBoundBox) BoundBox();

    for(int i = 0; i < node->totalSurfaces; ++i)
        x_bspnode_calculate_geo_boundbox_add_surface(node, node->firstSurface + i, level);
    
    const float SNAP = 16;
    node->geoBoundBox.v[0].x = floor(node->geoBoundBox.v[0].x / SNAP) * SNAP;
    node->geoBoundBox.v[0].y = floor(node->geoBoundBox.v[0].y / SNAP) * SNAP;
    node->geoBoundBox.v[0].z = floor(node->geoBoundBox.v[0].z / SNAP) * SNAP;
    
    node->geoBoundBox.v[1].x = ceil(node->geoBoundBox.v[1].x / SNAP) * SNAP;
    node->geoBoundBox.v[1].y = ceil(node->geoBoundBox.v[1].y / SNAP) * SNAP;
    node->geoBoundBox.v[1].z = ceil(node->geoBoundBox.v[1].z / SNAP) * SNAP;
    
    for(int i = 0; i < 2; ++i)
        node->geoBoundBox.v[i] = x_vec3_int_to_vec3(node->geoBoundBox.v + i);
    
    x_bspnode_calculate_geo_boundbox(node->frontChild, level);
    x_bspnode_calculate_geo_boundbox(node->backChild, level);
}

static void x_bsplevel_init_lightmap_data(BspLevel* level, X_BspLevelLoader* loader)
{
    level->lightmapData = loader->lightmapData.elem;
    loader->lightmapData.elem = NULL;
}

static void x_bsplevel_init_clipnodes(BspLevel* level, X_BspLevelLoader* loader)
{
    level->clipNodes = loader->clipNodes.elem;
    level->totalClipNodes = loader->clipNodes.count;

    loader->clipNodes.elem = nullptr;
}

static void x_bsplevel_init_collision_hulls(BspLevel* level, X_BspLevelLoader* loader)
{
    for(int i = 0; i < X_BSPLEVEL_MAX_COLLISION_HULLS; ++i)
        level->collisionHulls[i] = loader->collisionHulls[i];
}

static void x_bsplevel_init_entity_dictionary(BspLevel* level, X_BspLevelLoader* loader)
{
    level->entityDictionary = loader->entityDictionary;
    loader->entityDictionary = NULL;
}

static void x_bsplevel_init_from_bsplevel_loader(BspLevel* level, X_BspLevelLoader* loader)
{
    x_bsplevel_allocate_memory(level, loader);
    
    x_bsplevel_init_entity_dictionary(level, loader);
    x_bsplevel_init_pvs(level, loader);
    x_bsplevel_init_lightmap_data(level, loader);
    x_bsplevel_init_vertices(level, loader);
    x_bsplevel_init_edges(level, loader);
    x_bsplevel_init_planes(level, loader);
    x_bsplevel_init_marksurfaces(level, loader);
    x_bsplevel_init_leaves(level, loader);
    x_bsplevel_init_nodes(level, loader);
    x_bsplevel_init_clipnodes(level, loader);
    x_bsplevel_init_surfacedgeids(level, loader);
    x_bsplevel_init_models(level, loader);
    x_bsplevel_init_textures(level, loader);
    x_bsplevel_init_facetextures(level, loader);
    x_bsplevel_init_surfaces(level, loader);
    
    x_bsplevel_init_collision_hulls(level, loader);
    
    X_BspNode* levelRootNode = x_bsplevel_get_root_node(level);
    x_bspnode_calculate_geo_boundbox(levelRootNode, level);
    
    level->pvs.updatePvsData();
}

static bool x_bsplevelloader_load_bsp_file(X_BspLevelLoader* loader, const char* fileName, EngineQueue* engineQueue)
{
    Log::info("Loading map %s", fileName);

    loader->engineQueue = engineQueue;

    loader->progressEvent.type = EVENT_LEVEL_LOAD_PROGRESS;
    loader->progressEvent.levelLoadProgress.currentStep = 0;
    loader->progressEvent.levelLoadProgress.totalSteps = totalSteps;
    loader->progressEvent.fileName = fileName;

    loader->sendProgressEvent();

    if(!x_file_open_reading(&loader->file, fileName))
    {
        x_log_error("Failed to open BSP file %s\n", fileName);
        return 0;
    }
    
    x_bsploaderheader_read_from_file(&loader->header, &loader->file);
    x_log("BSP version: %d", loader->header.bspVersion);
    
    if(loader->header.bspVersion != 29)
        return 0;
    
    x_bsplevelloader_load_entity_dictionary(loader);

    loader->loadLump(X_LUMP_VISIBILITY, loader->compressedPvsData, "pvs size");
    loader->loadLump(X_LUMP_LIGHTING, loader->lightmapData, "lightmap size");
    loader->loadLump(X_LUMP_PLANES, loader->planes, "planes");
    loader->loadLump(X_LUMP_VERTEXES, loader->vertices, "vertices");
    loader->loadLump(X_LUMP_EDGES, loader->edges, "edges");
    loader->loadLump(X_LUMP_FACES, loader->faces, "faces");
    loader->loadLump(X_LUMP_LEAFS, loader->leaves, "leaves");
    loader->loadLump(X_LUMP_NODES, loader->nodes, "nodes");
    loader->loadLump(X_LUMP_MODELS, loader->models, "models");
    loader->loadLump(X_LUMP_MARKSURFACES, loader->markSurfaces, "marksurfaces");
    loader->loadLump(X_LUMP_SURFEDGES, loader->surfaceEdgeIds, "surface edge ids");

    x_bsplevelloader_load_textures(loader);
    x_bsplevelloader_load_facetextures(loader);

    loader->loadLump(X_LUMP_CLIPNODES, loader->clipNodes, "clip nodes");
    
    x_bsplevelloader_init_collision_hulls(loader);
    
    return 1;
}

static void x_bsplevelloader_cleanup(X_BspLevelLoader* level)
{
    x_free(level->compressedPvsData.elem);
    x_free(level->lightmapData.elem);
    x_free(level->edges.elem);
    x_free(level->faces.elem);
    x_free(level->faceTextures);
    x_free(level->leaves.elem);
    x_free(level->markSurfaces.elem);
    x_free(level->models.elem);
    x_free(level->nodes.elem);
    x_free(level->planes.elem);
    x_free(level->surfaceEdgeIds.elem);
    x_free(level->textures);
    x_free(level->textureTexels);
    x_free(level->vertices.elem);
    x_free(level->clipNodes.elem);
    x_free(level->entityDictionary);
    
    x_file_close(&level->file);
}

bool x_bsplevel_load_from_bsp_file(BspLevel* level, const char* fileName, EngineQueue* engineQueue)
{
    X_BspLevelLoader loader;
    if(!x_bsplevelloader_load_bsp_file(&loader, fileName, engineQueue))
        return 0;
    
    x_bsplevel_init_from_bsplevel_loader(level, &loader);
    x_bsplevelloader_cleanup(&loader);
    
    char mapName[X_FILENAME_MAX_LENGTH];
    x_filepath_extract_filename(fileName, mapName);
    x_strncpy(level->name, mapName, X_BSPLEVEL_MAX_NAME_LENGTH);
    
    level->flags = X_BSPLEVEL_LOADED;

    level->portalHead = nullptr;
    
    return 1;
}

