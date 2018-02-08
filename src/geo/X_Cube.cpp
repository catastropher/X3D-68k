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

#include "X_Cube.h"
#include "engine/X_EngineContext.h"
#include "X_Ray3.h"
#include "X_Polygon3.h"

void x_cube_init(X_Cube* cube, int width, int height, int depth)
{
    width <<= 16;
    height <<= 16;
    depth <<= 16;
    
    cube->vertices[0] = x_vec3_make(width, -height, -depth);
    cube->vertices[1] = x_vec3_make(width, -height, depth);
    cube->vertices[2] = x_vec3_make(-width, -height, depth);
    cube->vertices[3] = x_vec3_make(-width, -height, -depth);
    
    for(int i = 0; i < 4; ++i)
        cube->vertices[i + 4] = x_vec3_make(cube->vertices[i].x, -cube->vertices[i].y, cube->vertices[i].z);
}

void x_cube_translate(X_Cube* cube, X_Vec3 translation)
{
    for(int i = 0; i < 8; ++i)
        cube->vertices[i] = x_vec3_add(cube->vertices + i, &translation);
}

void x_cube_render(const X_Cube* cube, X_RenderContext* rcontext, X_Color color)
{    
    for(int i = 0; i < 4; ++i)
    {   
        int nextVertex = (i != 3 ? i + 1 : 0);
        
        X_Ray3 topRay = x_ray3_make(cube->vertices[i], cube->vertices[nextVertex]);
        X_Ray3 bottomRay = x_ray3_make(cube->vertices[i + 4], cube->vertices[nextVertex + 4]);
        X_Ray3 sideRay = x_ray3_make(cube->vertices[i], cube->vertices[i + 4]);
        
        x_ray3_render(&topRay, rcontext, color);
        x_ray3_render(&bottomRay, rcontext, color);
        x_ray3_render(&sideRay, rcontext, color);
    }
}

void x_cube_transform(const X_Cube* src, X_Cube* dest, const X_Mat4x4* mat)
{
    for(int i = 0; i < 8; ++i)
    {
        x_mat4x4_transform_vec3(mat, src->vertices + i, dest->vertices + i);
    }
}

void x_cube_get_face(const X_Cube* cube, int faceId, Polygon3* dest)
{
    dest->totalVertices = 4;
    
    if(faceId == 0)
    {
        for(int i = 0; i < 4; ++i)
            dest->vertices[3 - i] = cube->vertices[i];
    }
    else if(faceId == 1)
    {
         for(int i = 0; i < 4; ++i)
            dest->vertices[i] = cube->vertices[i + 4];
    }
    else
    {
        int sideId = faceId - 2;
        int next = (sideId != 3 ? sideId + 1 : 0);
        dest->vertices[0] = cube->vertices[sideId];
        dest->vertices[1] = cube->vertices[next];
        dest->vertices[2] = cube->vertices[next + 4];
        dest->vertices[3] = cube->vertices[sideId + 4];
    }
}

void x_cube_get_faces_containing_vertex(const X_Cube* cube, int vertexId, int faceDest[3])
{
    if(vertexId < 4)
        faceDest[0] = 0;
    else
        faceDest[0] = 1;
    
    int side = (vertexId < 4 ? vertexId : vertexId - 4);
    int prev = (side != 0 ? side - 1 : 3);
    
    faceDest[1] = side + 2;
    faceDest[2] = prev + 2;
}

