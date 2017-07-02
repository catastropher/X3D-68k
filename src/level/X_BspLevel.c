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

static void x_bspedge_read_from_file(X_BspEdge* edge, X_File* file)
{
    edge->v[0] = x_file_read_le_int16(file);
    edge->v[1] = x_file_read_le_int16(file);
}

static void x_bsplevel_load_planes(X_BspLevel* level, X_File* file)
{
    const int PLANE_SIZE_IN_FILE = 20;
    X_BspLump* planeLump = level->header.lumps + X_LUMP_PLANES;
    int totalPlanes = planeLump->length / PLANE_SIZE_IN_FILE;
    
    x_file_seek(file, planeLump->fileOffset);
    
    x_log("Total planes: %d\n", totalPlanes);
    
    for(int i = 0; i < totalPlanes; ++i)
    {
        X_BspPlane p;
        x_bspplane_read_from_file(&p, file);
    }
}

static void x_bsplevel_load_vertices(X_BspLevel* level, X_File* file)
{
    const int VERTEX_SIZE_IN_FILE = 12;
    X_BspLump* vertexLump = level->header.lumps + X_LUMP_VERTEXES;
    
    level->totalVertices = vertexLump->length / VERTEX_SIZE_IN_FILE;
    level->vertices = x_malloc(level->totalVertices * sizeof(X_BspVertex));
    
    x_log("Total vertices: %d\n", level->totalVertices);
    
    x_file_seek(file, vertexLump->fileOffset);
    
    for(int i = 0; i < level->totalVertices; ++i)
    {
        x_bspvertex_read_from_file(level->vertices + i, file);
    }
}

static void x_bsplevel_load_edges(X_BspLevel* level, X_File* file)
{
    const int EDGE_SIZE_IN_FILE = 4;
    X_BspLump* edgeLump = level->header.lumps + X_LUMP_EDGES;
    
    level->totalEdges = edgeLump->length / EDGE_SIZE_IN_FILE;
    level->edges = x_malloc(level->totalEdges * sizeof(X_BspVertex));
    
    x_log("Total edges: %d\n", level->totalEdges);
    
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
    
    x_log("BSP version: %d\n", level->header.bspVersion);
    
    x_bspheader_read_from_file(&level->header, &file);
    x_bsplevel_load_planes(level, &file);
    x_bsplevel_load_vertices(level, &file);
    x_bsplevel_load_edges(level, &file);
    
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

