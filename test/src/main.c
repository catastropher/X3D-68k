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

// Only here for testing purposes
typedef struct Portal
{
    X_Vec3 geometryVertices[4];
    X_Polygon3 geometry;
    _Bool portalOnWall;
    X_Mat4x4 wallOrientation;
} Portal;

void portal_init(Portal* portal, X_Vec3 center, X_Mat4x4* wallOrientation)
{
    x_fp16x16 w = x_fp16x16_from_int(50);
    x_fp16x16 h = x_fp16x16_from_int(50);
    
    X_Vec3 v[4] = 
    {
        x_vec3_make(-w, -h, 0),
        x_vec3_make(-w, h, 0),
        x_vec3_make(w, h, 0),
        x_vec3_make(w, -h, 0)
    };
    
    portal->geometry = x_polygon3_make(portal->geometryVertices, 4);
    
    for(int i = 0; i < 4; ++i)
    {
        x_mat4x4_transform_vec3(wallOrientation, v + i, portal->geometryVertices + i);
        portal->geometryVertices[i] = x_vec3_add(portal->geometryVertices + i, &center);
    }
    
    portal->portalOnWall = 1;
    portal->wallOrientation = *wallOrientation;
}

static void shoot_portal(Portal* portal, X_EngineContext* engineContext, X_CameraObject* cam)
{
    X_Vec3 camPos = x_cameraobject_get_position(cam);
        
    X_Vec3 forward, up, right;
    x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);
    
    X_Vec3 end = x_vec3_add_scaled(&camPos, &forward, x_fp16x16_from_float(3000));
    
    X_RayTracer trace;
    x_raytracer_init(&trace, &engineContext->currentLevel, x_bsplevel_get_level_model(&engineContext->currentLevel), &camPos, &end, NULL);
    trace.rootClipNode = 0;
    
    if(!x_raytracer_trace(&trace))
        return;
    
    X_Mat4x4 wallOrientation;
    x_plane_get_orientation(&trace.collisionPlane, cam, &wallOrientation);
    portal_init(portal, trace.collisionPoint, &wallOrientation);
}

void handle_test_portal(Portal* portal, X_EngineContext* engineContext, X_CameraObject* cam)
{
    X_RenderContext renderContext;
    x_enginecontext_get_rendercontext_for_camera(engineContext, cam, &renderContext);
    
    --renderContext.currentFrame;
    
    //x_bsplevel_render_submodels(&engineContext->currentLevel, &renderContext);
    
    if(x_keystate_key_down(&engineContext->keystate, 'k'))
        shoot_portal(portal, engineContext, cam);
    
    if(portal->portalOnWall)
        x_polygon3_render_wireframe(&portal->geometry, &renderContext, 255);
    
    x_mat4x4_visualize(&portal->wallOrientation, x_vec3_origin(), &renderContext);
}

X_Texture paint;

void apply_paint_to_surface(X_BspSurface* surface, X_Vec3* pos, X_EngineContext* engineContext)
{
    X_Vec3* normal = &surface->plane->plane.normal;
    X_Vec3 offset = x_vec3_scale(normal, x_plane_point_distance(&surface->plane->plane, pos));
    X_Vec3 pointOnPlane = x_vec3_sub(pos, &offset);
    
    X_BspFaceTexture* tex = surface->faceTexture;
    
    
    for(int i = 0; i < 4; ++i)
    {
        int u = x_fp16x16_to_int(x_vec3_dot(&pointOnPlane, &tex->uOrientation) + tex->uOffset - surface->textureMinCoord.x);
        int v = x_fp16x16_to_int(x_vec3_dot(&pointOnPlane, &tex->vOrientation) + tex->vOffset - surface->textureMinCoord.y);
        
        X_Vec2_fp16x16 uOrientation = x_vec2_make(X_FP16x16_ONE >> i, 0);
        X_Vec2_fp16x16 vOrientation = x_vec2_make(0, X_FP16x16_ONE >> i);
        
        X_Texture surfaceTex;
        x_bspsurface_get_surface_texture_for_mip_level(surface, i, &engineContext->renderer, &surfaceTex);
        
        x_texture_draw_decal(&surfaceTex, &paint, x_vec2_make(u >> i, v >> i), &uOrientation, &vOrientation, x_palette_get_quake_palette()->lightBlue);
    }
}

void apply_paint_to_node(X_BspNode* node, X_Vec3* pos, X_EngineContext* engineContext)
{
    
    for(int i = 0; i < node->totalSurfaces; ++i)
    {
        apply_paint_to_surface(node->firstSurface + i, pos, engineContext);
    }
}

void apply_paint(X_EngineContext* engineContext, X_CameraObject* cam)
{
    X_BoundSphere sphere;
    sphere.center = x_cameraobject_get_position(cam);
    sphere.radius = x_fp16x16_from_int(32);
    
    X_BspNode* nodes[100];
    
    int totalNodes = x_bsplevel_find_nodes_intersecting_sphere(&engineContext->currentLevel, &sphere, nodes);
    
    for(int i = 0; i < totalNodes; ++i)
        apply_paint_to_node(nodes[i], &sphere.center, engineContext);
}

void fill_with_checkerboard(X_Texture* tex)
{
    int size = 8;
    
    for(int i = 0; i < tex->h; ++i)
    {
        for(int j = 0; j < tex->w; ++j)
        {
            int x = j / size;
            int y = i / size;
            X_Color color;
            
            if((x % 2) == (y % 2))
                color = x_palette_get_quake_palette()->white;
            else
                color = x_palette_get_quake_palette()->black;
            
            x_texture_set_texel(tex, j, i, color);
        }
    }
}

void fill_circle(X_Texture* tex, X_Color color, X_Color background)
{    
    int r = X_MIN(tex->w, tex->h) / 2;
    
    for(int i = 0; i < tex->h; ++i)
    {
        for(int j = 0; j < tex->w; ++j)
        {
            int dx = j - r;
            int dy = i - r;
            
            if(dx * dx + dy * dy < r * r)
                x_texture_set_texel(tex, j, i, color);
            else
                x_texture_set_texel(tex, j, i, background);
        }
    }
}

void add_count(X_Texture* tex, int x, int y, int* r, int* g, int* b, int* total)
{
    if(x < 0 || x >= tex->w || y < 0 || y >= tex->h)
        return;
    
    unsigned char rr, gg, bb;
    x_palette_get_rgb(x_palette_get_quake_palette(), x_texture_get_texel(tex, x, y), &rr, &gg, &bb);
    
    *r += rr;
    *g += gg;
    *b += bb;
    *total += 1;
}

void shrink_antialias(X_Texture* tex, X_Texture* dest)
{
    int scale = tex->w / dest->w;
    
    for(int i = 0; i < dest->h; ++i)
    {
        for(int j = 0; j < dest->w; ++j)
        {
            int r = 0, g = 0, b = 0, total = 0;
            
            for(int k = -scale / 2; k <= scale / 2; ++k)
            {
                for(int d = -scale / 2; d <= scale / 2; ++d)
                {
                    add_count(tex, j * scale + d, i * scale + k, &r, &g, &b, &total);
                }
            }
            
            x_texture_set_texel(dest, j, i, x_palette_get_closest_color_from_rgb(x_palette_get_quake_palette(), r / total, g / total, b / total));
        }
    }
}

X_BoxCollider cube;

void init_cube()
{
    static X_BoundBox box;
    x_boxcollider_init(&cube, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    cube.position = x_vec3_origin();
    cube.position.y -= x_fp16x16_from_int(100);
    cube.velocity = x_vec3_origin();
}

void update_cube(X_BspLevel* level)
{
    x_boxcollider_update(&cube, level);
}

void draw_cube(X_RenderContext* renderContext)
{
    X_Cube geo;
    x_cube_init(&geo, 32, 32, 32);
    x_cube_translate(&geo, cube.position);
    x_cube_render(&geo, renderContext, 255);
}

void gameloop(Context* context)
{
    Portal portal;
    portal.portalOnWall = 0;
    x_mat4x4_load_identity(&portal.wallOrientation);
    
    X_Texture tex;
    x_bsplevel_get_texture(&context->engineContext->currentLevel, 0, 0, &tex);
    
    x_gameobjectloader_load_objects(context->engineContext, context->engineContext->currentLevel.entityDictionary);
    
    x_texture_init(&paint, 32, 32);
    fill_circle(&paint, x_palette_get_quake_palette()->darkBlue, x_palette_get_quake_palette()->lightBlue);
    
    //fill_with_checkerboard(&paint);
    
    init_cube();
    
    while(!context->quit)
    {
        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(context->engineContext, context->cam, &renderContext);
        
        render(context);
        handle_test_portal(&portal, context->engineContext, context->cam);
        
        //apply_paint(context->engineContext, context->cam);
        
        if(x_keystate_key_down(&context->engineContext->keystate, '/'))
        {
            cube.position = x_cameraobject_get_position(context->cam);
        }
        
        update_cube(renderContext.level);
        draw_cube(&renderContext);
        
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

