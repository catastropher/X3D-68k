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
    x_raytracer_init(&trace, &engineContext->currentLevel, &camPos, &end, NULL);
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
    
    if(x_keystate_key_down(&engineContext->keystate, 'k'))
        shoot_portal(portal, engineContext, cam);
    
    if(portal->portalOnWall)
        x_polygon3_render_wireframe(&portal->geometry, &renderContext, 255);
    
    x_mat4x4_visualize(&portal->wallOrientation, x_vec3_origin(), &renderContext);
}

void gameloop(Context* context)
{
    Portal portal;
    portal.portalOnWall = 0;
    x_mat4x4_load_identity(&portal.wallOrientation);
    
    while(!context->quit)
    {
        render(context);
        handle_test_portal(&portal, context->engineContext, context->cam);
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

