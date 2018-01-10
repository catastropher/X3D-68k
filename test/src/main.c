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

typedef struct CubeObject
{
    X_GameObject base;
    X_BoxCollider collider;
    X_BspModel* model;
    X_Vec3 center;
} CubeObject;

void cube_update(X_GameObject* obj, x_fp16x16 deltaTime)
{
    CubeObject* cube = (CubeObject*)obj;
    x_boxcollider_update(&cube->collider, &obj->engineContext->currentLevel);
    
    cube->model->origin = x_vec3_sub(&cube->collider.position, &cube->center);
    cube->model->origin.y += x_fp16x16_from_int(8);
    
    if(x_keystate_key_down(&obj->engineContext->keystate, '/'))
    {
        cube->collider.position = x_cameraobject_get_position(obj->engineContext->screen.cameraListHead);
    }
        
}

X_GameObject* cube_new(X_EngineContext* engineContext, X_Edict* edict);

static X_GameObjectType g_cubeType = 
{
    .typeId = 1,
    .name = "cube",
    .handlers = 
    {
        .createNew = cube_new,
        .update = cube_update
    }
};


X_GameObject* cube_new(X_EngineContext* engineContext, X_Edict* edict)
{
    CubeObject* cube = (CubeObject*)x_gameobject_new(engineContext, sizeof(CubeObject));
    
    cube->model = engineContext->currentLevel.models + x_edict_get_model_id(edict, "model");
    
    static X_BoundBox box;
    x_boxcollider_init(&cube->collider, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    
    cube->center.x = (cube->model->boundBox.v[1].x + cube->model->boundBox.v[0].x) / 2;
    cube->center.y = (cube->model->boundBox.v[1].y + cube->model->boundBox.v[0].y) / 2;
    cube->center.z = (cube->model->boundBox.v[1].z + cube->model->boundBox.v[0].z) / 2;
    
    x_gameobject_activate((X_GameObject*)cube);
    cube->base.type = &g_cubeType;
    
    cube->collider.position = cube->center;
    
    return (X_GameObject*)cube;
}

static void cmd_trigger(X_EngineContext* engineContext, int argc, char* argv[])
{
    if(argc != 3)
        return;
    
    x_gameobject_trigger(&engineContext->screen.cameraListHead->base, argv[1], atoi(argv[2]));
}

void test_socket();

X_Socket serverSocket;
_Bool isConnected = 0;

_Bool sendPacket;

void cmd_packet(X_EngineContext* context, int argc, char* argv[])
{
    sendPacket = 1;
}

void send_update_packet(Context* context)
{
//     if(!sendPacket)
//         return;
//     
//     sendPacket = 0;
    
    char buf[240];
    X_Packet packet;
    x_packet_init(&packet, X_PACKET_DATA, buf, 0);
    
    X_CameraObject* cam = context->cam;
    X_Vec3 pos = x_cameraobject_get_position(cam);
    
    x_packet_write_short(&packet, x_fp16x16_to_int(pos.x));
    x_packet_write_short(&packet, x_fp16x16_to_int(pos.y));
    x_packet_write_short(&packet, x_fp16x16_to_int(pos.z));
    
    x_packet_write_byte(&packet, x_fp16x16_to_int(cam->angleX));
    x_packet_write_byte(&packet, x_fp16x16_to_int(cam->angleY));
    
    x_socket_send_packet(&serverSocket, &packet);
    
#ifdef __pc__
    usleep(100 * 1000);
#endif
}

_Bool recv_update_packet(Context* context)
{
    //x_log("Recv... %d", serverSocket.queueHead);
    X_Packet* packet = x_socket_receive_packet(&serverSocket);
    if(!packet)
        return 0;
    
    packet->readPos = 0;
    
    X_CameraObject* cam = context->cam;
    
    cam->collider.position.x = (int)x_packet_read_short(packet) << 16;
    cam->collider.position.y = (int)x_packet_read_short(packet) << 16;
    cam->collider.position.z = (int)x_packet_read_short(packet) << 16;
    
    cam->angleX = (int)x_packet_read_byte(packet) << 16;
    cam->angleY = (int)x_packet_read_byte(packet) << 16;
    
    //x_vec3_fp16x16_print(&cam->collider.position, "\tRecv pos");
    
    x_cameraobject_update_view(cam);
    return 1;
}

void handle_net_nspire(Context* context)
{
    if(!isConnected)
        return;
    
    while(recv_update_packet(context)) ;
}


void handle_net_pc(Context* context)
{
    X_ConnectRequest request;
    if(x_net_get_connect_request(&request))
    {
        x_log("Has request!");
        isConnected = x_socket_open(&serverSocket, request.address);
    }
    
    if(!isConnected)
        return;
    
    send_update_packet(context);
}

void handle_net(Context* context)
{
#ifdef __pc__
    handle_net_pc(context);
#else
    handle_net_nspire(context);
#endif
}

void gameloop(Context* context)
{
    x_console_register_cmd(&context->engineContext->console, "trigger", cmd_trigger);
    
    Portal portal;
    portal.portalOnWall = 0;
    x_mat4x4_load_identity(&portal.wallOrientation);
    
    x_texture_init(&paint, 32, 32);
    fill_circle(&paint, x_palette_get_quake_palette()->darkBlue, x_palette_get_quake_palette()->lightBlue);
    
    //fill_with_checkerboard(&paint);
    
    x_objectfactory_register_type(&context->engineContext->gameObjectFactory, &g_cubeType);
    
    x_console_execute_cmd(&context->engineContext->console, "map portal2");
    x_gameobjectloader_load_objects(context->engineContext, context->engineContext->currentLevel.entityDictionary);
    
    while(!context->quit)
    {
        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(context->engineContext, context->cam, &renderContext);
        
        render(context);

        handle_net(context);
        
        handle_keys(context);
        screen_update(context);
    }    
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, argv[0]);
    
    x_console_register_cmd(&context.engineContext->console, "packet", cmd_packet);
   
    gameloop(&context);
    
//     test_socket();
//     
// #ifdef __nspire__
//     if(x_socket_open(&serverSocket, "calc:0"))
//     {
//         isConnected = 1;
//         gameloop(&context);
//     }
//     
// #else
//     gameloop(&context);
// #endif
//     
    cleanup(&context);
}

