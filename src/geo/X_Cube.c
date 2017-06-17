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

void x_cube_init(X_Cube* cube, int width, int height, int depth)
{
    cube->vertices[0] = x_vec3_make(width, height, -depth);
    cube->vertices[1] = x_vec3_make(width, height, depth);
    cube->vertices[2] = x_vec3_make(-width, height, depth);
    cube->vertices[3] = x_vec3_make(-width, height, -depth);
    
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
    X_Vec2 projectedV[8];
    
    for(int i = 0; i < 8; ++i)
        x_viewport_project(&rcontext->cam->viewport, cube->vertices + i, projectedV + i);
    
    for(int i = 0; i < 4; ++i)
    {   
        int next = (i != 3 ? i + 1 : 0);
        
        x_canvas_draw_line(rcontext->canvas, projectedV[i], projectedV[next], color);
        x_canvas_draw_line(rcontext->canvas, projectedV[i + 4], projectedV[next + 4], color);
        x_canvas_draw_line(rcontext->canvas, projectedV[i], projectedV[i + 4], color);
    }
}

void x_cube_transform(const X_Cube* src, X_Cube* dest, const X_Mat4x4* mat)
{
    for(int i = 0; i < 8; ++i)
    {
        x_mat4x4_transform_vec3(mat, src->vertices + i, dest->vertices + i);
    }
}

