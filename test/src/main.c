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

#include <X3D/X3D.h>
#include <SDL/SDL.h>
#include <unistd.h>
#include <math.h>

#include "Context.h"
#include "screen.h"
#include "keys.h"
#include "init.h"
#include "render.h"

X_Vec3_int portalV[4];
X_Polygon3 p = { 4, portalV};
_Bool portalOnWall;

void plane_get_orientation(X_Plane* plane, X_Mat4x4* dest, X_CameraObject* cam)
{
    X_Vec3 temp = plane->normal;
    temp.y = 0;
    
    X_Mat4x4 mat;
    x_mat4x4_load_y_rotation(&mat, X_ANG_270);
    
    X_Vec3 right, up;
    
    if(abs(plane->normal.y) != X_FP16x16_ONE)
    {
        x_mat4x4_transform_vec3(&mat, &temp, &right);
        x_vec3_normalize(&right);
        
        up = x_vec3_cross(&plane->normal, &right);
    }
    else
    {
        // Pick the vectors from the cam direction
        X_Vec3 temp;
        x_mat4x4_extract_view_vectors(&cam->viewMatrix, &up, &right, &temp);
        
        right.y = 0;
        x_vec3_normalize(&right);
        
        up.y = 0;
        x_vec3_normalize(&up);
    }
    
    x_mat4x4_load_identity(dest);
    
    X_Vec4 up4 = x_vec4_from_vec3(&up);
    X_Vec4 right4 = x_vec4_from_vec3(&right);
    X_Vec4 forward4 = x_vec4_from_vec3(&plane->normal);
    
    x_mat4x4_set_column(dest, 0, &right4);
    x_mat4x4_set_column(dest, 1, &up4);
    x_mat4x4_set_column(dest, 2, &forward4);
}

void create_portal(X_Mat4x4* mat, X_Vec3 center)
{
    int w = 50;
    int h = 50;
    
    X_Vec3_int v[4] = 
    {
        x_vec3_make(-w, -h, 0),
        x_vec3_make(-w, h, 0),
        x_vec3_make(w, h, 0),
        x_vec3_make(w, -h, 0)
    };

    X_Vec3_int c = x_vec3_to_vec3_int(&center);
    
    for(int i = 0; i < 4; ++i)
    {
        //x_mat4x4_transform_vec3(mat, v + i, portalV + i);
        portalV[i] = x_vec3_add(portalV + i, &c);
    }
    
    portalOnWall = 1;
}

void mat4x4_visualize(X_Mat4x4* mat, X_RenderContext* renderContext)
{
    const X_Palette* p = x_palette_get_quake_palette();
    X_Color color[] = { p->brightRed, p->lightGreen, p->lightBlue };
    
    for(int i = 0; i < 3; ++i)
    {
        X_Vec4 v;
        x_mat4x4_get_column(mat, i, &v);
        
        X_Vec3 v3 = x_vec4_to_vec3(&v);
        
        X_Vec3 end = x_vec3_scale_int(&v3, 50);
        
        X_Ray3 r = x_ray3_make(x_vec3_origin(), x_vec3_to_vec3_int(&end));
        x_ray3_render(&r, renderContext, color[i]);
    }
}


const char* x_game_name(void)
{
    return "X3D Test";
}

int x_game_major_version(void)
{
    return 0;
}

int x_game_minor_version(void)
{
    return 1;
}

void gameloop(Context* context)
{
    X_Mat4x4 identity;
    x_mat4x4_load_identity(&identity);
    
    while(!context->quit)
    {
        render(context);
        

        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(context->engineContext, context->cam, &renderContext);
        
        X_BoundSphere sphere = { x_cameraobject_get_position(context->cam), x_fp16x16_from_int(30) };
        
        X_BspNode* nodes[100];
        int total = x_bsplevel_find_nodes_intersecting_sphere(renderContext.level, &sphere, nodes + 0);

        for(int i = 0; i < total; ++i)
        {
            X_BspNode* node = nodes[i];
            
            for(int j = 0; j < node->totalSurfaces; ++j) {
                X_BspSurface* s = node->firstSurface + j;
                
                X_Texture tex;
                
                for(int k = 0; k < 4; ++k)
                {
                    x_bspsurface_get_surface_texture_for_mip_level(s, k, renderContext.renderer, &tex);
                    
                    //memset(tex.texels, 255, tex.w * tex.h);
                }
            }
        }
        
        if(x_keystate_key_down(&context->engineContext->keystate, 'k'))
        {
            X_Vec3 camPos = x_cameraobject_get_position(context->cam);
            
            X_Vec3 forward, up, right;
            x_mat4x4_extract_view_vectors(&context->cam->viewMatrix, &forward, &right, &up);
            
            X_Vec3 end = x_vec3_add_scaled(&camPos, &forward, x_fp16x16_from_float(3000));
            
            X_RayTracer trace;
            x_raytracer_init(&trace, &context->engineContext->currentLevel, &camPos, &end, NULL);
            trace.rootClipNode = 0;
            
            if(x_raytracer_trace(&trace))
            {
                plane_get_orientation(&trace.collisionPlane, &identity, context->cam);
                
                create_portal(&identity, trace.collisionPoint);
            }
            
        }
        
        if(portalOnWall)
        {
            x_polygon3_render_wireframe(&p, &renderContext, 255);
        }
        
        mat4x4_visualize(&identity, &renderContext);
        
        handle_keys(context);
        screen_update(context);
    }    
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, argv[0]);    
    gameloop(&context);
    cleanup(&context);
}

