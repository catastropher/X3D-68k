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
#include "system/X_File.h"
#include "error/X_log.h"
#include "memory/X_alloc.h"
#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"

static X_Vec3_float x_bspfile_convert_coordinate(const X_Vec3_float* v)
{
    return x_vec3_float_make(v->y, -v->z, -v->x);
}

static void x_bsplump_read_from_file(X_BspLump* lump, X_File* file)
{
    lump->fileOffset = x_file_read_le_int32(file);
    lump->length = x_file_read_le_int32(file);
}

static void x_bspheader_read_from_file(X_BspHeader* header, X_File* file)
{
    header->bspVersion = x_file_read_le_int32(file);
    
    for(int i = 0; i < X_BSP_TOTAL_LUMPS; ++i)
        x_bsplump_read_from_file(header->lumps + i, file);
}

static void x_bspplane_read_from_file(X_BspPlane* plane, X_File* file)
{
    X_Vec3_float normal;
    x_file_read_vec3_float(file, &normal);
    normal = x_bspfile_convert_coordinate(&normal);
    
    plane->plane.normal = x_vec3_float_to_vec3_fp16x16(&normal);
    
    float dist = x_file_read_le_float32(file);
    plane->plane.d = x_fp16x16_from_float(dist);
    
    plane->type = x_file_read_le_int32(file);
}

static void x_bspvertex_read_from_file(X_BspVertex* vertex, X_File* file)
{
    X_Vec3_float v;
    x_file_read_vec3_float(file, &v);
    
    v = x_bspfile_convert_coordinate(&v);
    vertex->v = x_vec3_float_to_vec3(&v);
}

static void x_bspface_read_from_file(X_BspFace* face, X_File* file)
{
    face->planeNum = x_file_read_le_int16(file);
    face->side = x_file_read_le_int16(file);
    face->firstEdge = x_file_read_le_int32(file);
    face->texInfo = x_file_read_le_int16(file);
    
    for(int i = 0; i < X_BSPFACE_MAX_LIGHTMAPS; ++i)
        face->lightmapStypes[i] = x_file_read_char(file);
    
    face->lightmapOffset = x_file_read_le_int32(file);
}

static void x_bspleaf_read_from_file(X_BspLeaf* leaf, X_File* file)
{
    leaf->contents = x_file_read_be_int32(file);
    leaf->pvsOffset = x_file_read_be_int32(file);
    
    for(int i = 0; i < 3; ++i)
        leaf->mins[i] = x_file_read_le_int16(file);
    
    for(int i = 0; i < 3; ++i)
        leaf->maxs[i] = x_file_read_le_int16(file);
    
    for(int i = 0; i < X_BSPLEAF_TOTAL_AMBIENTS; ++i)
        leaf->ambientLevel[i] = x_file_read_char(file);
}

static void x_bspedge_read_from_file(X_BspEdge* edge, X_File* file)
{
    edge->v[0] = x_file_read_le_int16(file);
    edge->v[1] = x_file_read_le_int16(file);
}

static void x_bsplevel_load_planes(X_BspLevel* level, X_File* file)
{
    const int PLANE_SIZE_IN_FILE = 20;
    X_BspLump* planeLump = level->header.lumps + X_LUMP_PLANES;
    
    level->totalPlanes = planeLump->length / PLANE_SIZE_IN_FILE;
    level->planes = malloc(level->totalPlanes * sizeof(X_BspPlane));
    
    x_log("Total planes: %d", level->totalPlanes);
    
    x_file_seek(file, planeLump->fileOffset);
    
    for(int i = 0; i < level->totalPlanes; ++i)
    {
        x_bspplane_read_from_file(level->planes + i, file);
    }
}

static void x_bsplevel_load_vertices(X_BspLevel* level, X_File* file)
{
    const int VERTEX_SIZE_IN_FILE = 12;
    X_BspLump* vertexLump = level->header.lumps + X_LUMP_VERTEXES;
    
    level->totalVertices = vertexLump->length / VERTEX_SIZE_IN_FILE;
    level->vertices = x_malloc(level->totalVertices * sizeof(X_BspVertex));
    
    x_log("Total vertices: %d", level->totalVertices);
    
    x_file_seek(file, vertexLump->fileOffset);
    
    for(int i = 0; i < level->totalVertices; ++i)
    {
        x_bspvertex_read_from_file(level->vertices + i, file);
    }
}

static void x_bsplevel_load_faces(X_BspLevel* level, X_File* file)
{
    const int FACE_SIZE_IN_FILE = 20;
    X_BspLump* faceLump = level->header.lumps + X_LUMP_FACES;
    
    level->totalFaces = faceLump->length / FACE_SIZE_IN_FILE;
    level->faces = x_malloc(level->totalFaces * sizeof(X_BspFace));
    
    x_log("Total faces: %d", level->totalFaces);
    
    x_file_seek(file, faceLump->fileOffset);
    
    for(int i = 0; i < level->totalFaces; ++i)
    {
        x_bspface_read_from_file(level->faces + i, file);
    }
}

static void x_bsplevel_load_leaves(X_BspLevel* level, X_File* file)
{
    const int LEAF_SIZE_IN_FILE = 28;
    X_BspLump* leafLump = level->header.lumps + X_LUMP_LEAFS;
    
    level->totalLeaves = leafLump->length / LEAF_SIZE_IN_FILE;
    level->leaves = x_malloc(level->totalLeaves * sizeof(X_BspLeaf));
    
    x_log("Total leaves: %d", level->totalLeaves);
    
    x_file_seek(file, leafLump->fileOffset);
    
    for(int i = 0; i < level->totalLeaves; ++i)
    {
        x_bspleaf_read_from_file(level->leaves + i, file);
    }
}

static void x_bsplevel_load_edges(X_BspLevel* level, X_File* file)
{
    const int EDGE_SIZE_IN_FILE = 4;
    X_BspLump* edgeLump = level->header.lumps + X_LUMP_EDGES;
    
    level->totalEdges = edgeLump->length / EDGE_SIZE_IN_FILE;
    level->edges = x_malloc(level->totalEdges * sizeof(X_BspVertex));
    
    x_log("Total edges: %d", level->totalEdges);
    
    x_file_seek(file, edgeLump->fileOffset);
    
    for(int i = 0; i < level->totalEdges; ++i)
    {
        x_bspedge_read_from_file(level->edges + i, file);
    }
}

_Bool x_bsplevel_load_from_bsp_file(X_BspLevel* level, const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
    {
        x_log_error("Failed to open BSP file %s\n", fileName);
        return 0;
    }
    
    x_log("BSP version: %d", level->header.bspVersion);
    
    x_bspheader_read_from_file(&level->header, &file);
    x_bsplevel_load_planes(level, &file);
    x_bsplevel_load_vertices(level, &file);
    x_bsplevel_load_edges(level, &file);
    x_bsplevel_load_faces(level, &file);
    x_bsplevel_load_leaves(level, &file);
    
    x_file_close(&file);
    return 1;
}

void x_bsplevel_render_wireframe(X_BspLevel* level, X_RenderContext* rcontext, X_Color color)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        X_BspEdge* edge = level->edges + i;
        
        X_Ray3 ray = x_ray3_make
        (
            level->vertices[edge->v[0]].v,
            level->vertices[edge->v[1]].v
        );
        
        x_ray3d_render(&ray, rcontext, color);
    }
}

