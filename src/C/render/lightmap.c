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

#include "X3D_polygon.h"
#include "X3D_camera.h"
#include "X3D_render.h"
#include "X3D_enginestate.h"
#include "render/X3D_lightmap.h"

static inline void* advance_ptr(void* ptr, size_t dist) {
    return ptr + dist;
}

// Builds a planar projection of a polygon (disregards the least significant axis) and
// normalizes the result
void x3d_polygon3d_build_planar_projection(X3D_Polygon3D* poly, X3D_PlanarProjection* proj) {
    X3D_Plane plane;
    x3d_polygon3d_calculate_plane(poly, &plane);
    
    int16 px = abs(plane.normal.x);
    int16 py = abs(plane.normal.y);
    int16 pz = abs(plane.normal.z);
    
    int16* u_component;
    int16* v_component;
    
    if(px > py && px > pz) {
        proj->plane_type = X3D_PLANE_YZ;
        u_component = &poly->v[0].y;
        v_component = &poly->v[0].z;
    }
    else if(py > px && py > pz) {
        proj->plane_type = X3D_PLANE_XZ;
        u_component = &poly->v[0].x;
        v_component = &poly->v[0].z;
    }
    else {
        proj->plane_type = X3D_PLANE_XY;
        u_component = &poly->v[0].x;
        v_component = &poly->v[0].y;
    }
    
    int16 min_x = 0x7FFF;
    int16 max_x = -0x7FFF;
    
    int16 min_y = 0x7FFF;
    int16 max_y = -0x7FFF;
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        proj->poly.v[i].x = *u_component;
        proj->poly.v[i].y = *v_component;
        
        min_x = X3D_MIN(min_x, proj->poly.v[i].x);
        min_y = X3D_MIN(min_y, proj->poly.v[i].y);
        
        max_x = X3D_MAX(max_x, proj->poly.v[i].x);
        max_y = X3D_MAX(max_y, proj->poly.v[i].y);
        
        u_component = advance_ptr(u_component, sizeof(X3D_Vex3D));
        v_component = advance_ptr(v_component, sizeof(X3D_Vex3D));
    }
    
    proj->poly.total_v = poly->total_v;
    
    for(i = 0; i < poly->total_v; ++i) {
        proj->poly.v[i].x -= min_x;
        proj->poly.v[i].y -= min_y;
    }
    
    proj->min_x = min_x;
    proj->min_y = min_y;
    
    proj->max_x = max_x;
    proj->max_y = max_y;
    proj->poly_plane = plane;
}

void x3d_planarprojection_project_point(X3D_PlanarProjection* proj, X3D_Vex3D* v, X3D_Vex2D* dest) {
    if(proj->plane_type == X3D_PLANE_YZ) {
        dest->x = v->y;
        dest->y = v->z;
    }
    else if(proj->plane_type == X3D_PLANE_XZ) {
        dest->x = v->x;
        dest->y = v->z;
    }
    else {
        dest->x = v->x;
        dest->y = v->y;
    }
    
    dest->x = (dest->x - proj->min_x) / X3D_LIGHTMAP_SCALE;
    dest->y = (dest->y - proj->min_y) / X3D_LIGHTMAP_SCALE;
}

void x3d_planarprojection_unproject_point(X3D_PlanarProjection* proj, X3D_Vex2D* src, X3D_Vex3D* dest) {
    X3D_Vex2D v = { src->x * X3D_LIGHTMAP_SCALE + proj->min_x, src->y * X3D_LIGHTMAP_SCALE + proj->min_y };
    
    X3D_Vex3D_float n = {
        proj->poly_plane.normal.x / 32767.0,
        proj->poly_plane.normal.y / 32767.0,
        proj->poly_plane.normal.z / 32767.0
    };
    
    if(proj->plane_type == X3D_PLANE_YZ) {
        dest->x = -(n.y * v.x + n.z * v.y - proj->poly_plane.d) / n.x;
        dest->y = v.x;
        dest->z = v.y;
    }
    else if(proj->plane_type == X3D_PLANE_XZ) {
        dest->x = v.x;
        dest->y = - ( n.x * v.x + n.z * n.y - proj->poly_plane.d) / n.y;
        dest->z = v.y;
    }
    else {
        dest->x = v.x;
        dest->y = v.y;
        dest->z = - ( n.x * v.x + n.y * v.y - proj->poly_plane.d) / n.z;
    }
    
    //x3d_log(X3D_INFO, "%d %d %d", dest->x, dest->y, dest->z);
}

void x3d_convert_view_coord_to_world_coord(X3D_Vex3D* view, X3D_CameraObject* cam, X3D_Vex3D* dest) {
    X3D_Mat3x3 mat = cam->base.mat;
    x3d_mat3x3_transpose(&mat);
    
    X3D_Vex3D unrotated;
    
    x3d_vex3d_int16_rotate(&unrotated, view, &mat);
    
    X3D_Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    *dest = x3d_vex3d_add(&unrotated, &cam_pos);
}

void x3d_calculate_position_from_z_buffer(int16 screen_x, int16 screen_y, X3D_CameraObject* cam, X3D_Vex3D* dest) {
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderManager* renderman = x3d_rendermanager_get();
    
    float z = renderman->zbuf[(int32)screen_y * screenman->w + screen_x];
    
    float x = (screen_x - screenman->w / 2) * z / screenman->scale_x;
    float y = (screen_y - screenman->h / 2) * z / screenman->scale_x;
 
    X3D_Vex3D v = { x, y, z };

    x3d_convert_view_coord_to_world_coord(&v, cam, dest);
}

void x3d_render_from_perspective_of_spotlight(X3D_SpotLight* light, X3D_CameraObject* light_cam) {
    light_cam->base.base.pos.x = (int32)light->pos.x << 8;
    light_cam->base.base.pos.y = (int32)light->pos.y << 8;
    light_cam->base.base.pos.z = (int32)light->pos.z << 8;
    
    x3d_mat3x3_construct(&light_cam->base.mat, &light->orientation);
    
    X3D_RenderContext render_context = {
        .render_type = X3D_RENDER_ID_BUFFER
    };
    
    x3d_screen_zbuf_clear();
    x3d_render(light_cam, &render_context);
    
    X3D_Vex3D v;
    x3d_calculate_position_from_z_buffer(640 / 2, 480 / 2, light_cam, &v);    
}

extern X3D_Level* global_level;

void x3d_lightmap_init(X3D_LightMapContext* context, uint32 id, X3D_Polygon3D* poly) {
    X3D_PlanarProjection* proj = context->proj + id;
    
    proj->poly.v = alloca(1000);
    x3d_polygon3d_build_planar_projection(poly, proj);
    
    X3D_LightMap* map = context->maps + id;
    
    map->w = (proj->max_x - proj->min_x) / X3D_LIGHTMAP_SCALE + 1;
    map->h = (proj->max_y - proj->min_y) / X3D_LIGHTMAP_SCALE + 1;
    
    map->data = calloc(sizeof(uint8) * map->w * map->h, 1);
    context->set[id] = calloc(sizeof(_Bool) * map->w * map->h, 1);
}

void x3d_lightmapcontext_init(X3D_LightMapContext* context, X3D_Level* level) {
    context->total_maps = x3d_segfaceid_create(level->segs.total, 0);
    context->set = malloc(sizeof(_Bool *) * context->total_maps);
    context->maps = malloc(sizeof(X3D_LightMap) * context->total_maps);
    context->proj = malloc(sizeof(X3D_PlanarProjection) * context->total_maps);
    context->total_light = 0;
    context->level = level;
    
    X3D_Prism3D prism = { .v = alloca(1000) };
    X3D_Polygon3D poly = { .v = alloca(1000) };
    
    
    uint32 i, j;
    for(i = 0; i < context->total_maps; ++i)
        context->maps[i].data = NULL;
    
    
    for(i = 0; i < level->segs.total; ++i) {
        X3D_LevelSegment* seg = x3d_level_get_segmentptr(level, i);
        x3d_levelsegment_get_geometry(level, seg, &prism);
        
        for(j = 0; j < seg->base_v + 2; ++j) {
            uint32 id = x3d_segfaceid_create(i, j);
            x3d_prism3d_get_face(&prism, j, &poly);
            x3d_lightmap_init(context, id, &poly);
        }
    }
}

void x3d_lightmapcontext_reset_set_values(X3D_LightMapContext* context) {
    uint16 i;
    
    for(i = 0; i < context->total_maps; ++i) {
        if(!context->maps[i].data)
            continue;
            
        uint32 total = context->maps[i].w * context->maps[i].h;
        
        uint32 j;
        for(j = 0; j < total; ++j) {
            context->set[i][j] = X3D_FALSE;
        }
    }
}

_Bool x3d_lightmapcontext_already_set(X3D_LightMapContext* context, uint32 id, X3D_Vex2D pos) {
    X3D_LightMap* map = context->maps + id;
    return context->set[id][(uint32)pos.y * map->w + pos.x];
}

void x3d_lightmapcontext_apply_light(X3D_LightMapContext* context, uint16 id, X3D_Vex2D pos, uint8 value) {
    X3D_LightMap* map = context->maps + id;
    if(pos.x < 0 || pos.x >= map->w || pos.y < 0 || pos.y >= map->h)
        return;
    
    if(x3d_lightmapcontext_already_set(context, id, pos))
        return;
    
    
    context->set[id][(uint32)pos.y * map->w + pos.x] = X3D_TRUE;
    map->data[(uint32)pos.y * map->w + pos.x] = X3D_MIN((uint32)map->data[(uint32)pos.y * map->w + pos.x] + value, 255);
}

uint8 x3d_lightmap_get_value(X3D_LightMap* map, uint16 x, uint16 y) {
    if(x < 0 || x >= map->w || y < 0 || y >= map->h)
        return 0;
    
    return map->data[(uint32)y * map->w + x];
}

extern float* id_zbuf;

void x3d_lightmap_build(X3D_SpotLight* light, X3D_LightMapContext* context) {
    int32 i, j, k;
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_CameraObject light_cam;
    
    id_zbuf = malloc(sizeof(float) * screenman->w * screenman->h);
    
    for(i = 0; i < (uint32)screenman->w * screenman->h; ++i) {
        id_zbuf[i] = 0x7F7F;
    }
    
    x3d_lightmapcontext_reset_set_values(context);
    
    x3d_render_from_perspective_of_spotlight(light, &light_cam);
    
    for(i = 0; i < context->total_maps; ++i) {
        if(context->maps[i].data != NULL) {
            X3D_LightMap* map = context->maps + i;
            
            for(j = 0; j < map->h; ++j) {
                for(k = 0; k < map->w; ++k) {
                    X3D_Vex2D v2d = { k, j };
                    X3D_Vex3D v;
                    
                    x3d_planarprojection_unproject_point(&context->proj[i], &v2d, &v);
                    
                    X3D_Vex3D transformed;
                    X3D_Vex2D projected;
                    x3d_camera_transform_points(&light_cam, &v, 1, &transformed, &projected);
                    
                    if(projected.x >= 0 && projected.x < screenman->w && projected.y >= 0 && projected.y < screenman->h) {
                        float z = id_zbuf[(int32)projected.y * screenman->w + projected.x];
                        
                        if(transformed.z > 0 && transformed.z < z + 10) {
                            //printf("Transformed: %d -> %d\n", transformed.z, z);
                            x3d_lightmapcontext_apply_light(context, i, v2d, 32);
                        }
                    }
                }
            }
        }
    }
    
    free(id_zbuf);
    id_zbuf = NULL;
    
    
//     for(i = 0; i < screenman->h; ++i) {
//         for(j = 0; j < screenman->w; ++j) {
//             uint32 id = x3d_screen_get_internal_value(j, i);
//             
//             if(id == 0x7FFF) continue;
//             
//             X3D_PlanarProjection* proj = context->proj + id;
//             
//             X3D_Vex3D v;
//             x3d_calculate_position_from_z_buffer(j, i, &light_cam, &v);
//                 
//             X3D_Vex2D plane_v;
//             x3d_planarprojection_project_point(proj, &v, &plane_v);
//             
//             x3d_lightmapcontext_apply_light(context, id, plane_v, 32);
//         }
//     }
}

void x3d_lightmapcontext_cleanup(X3D_LightMapContext* context) {
    uint32 i;
    for(i = 0; i < context->total_light; ++i) {
        free(context->maps[i].data);
        free(context->set[i]);
    }
    
    free(context->maps);
    free(context->proj);
    free(context->set);
}

void x3d_lightmap_blit(X3D_LightMap* map) {
    uint32 i, j;

    if(!map->data) return;
    
    for(i = 0; i < map->h; ++i) {
        for(j = 0; j < map->w; ++j) {
            uint8 val = x3d_lightmap_get_value(map, j, i);
            x3d_screen_draw_pix(j, i, x3d_rgb_to_color(val, val, val));
        }
    }
}


void add_test_lights(X3D_LightMapContext* context) {
    {
        X3D_SpotLight light;
        
        light.pos = x3d_vex3d_make(0, 0, -200);
        light.orientation.y = 0;//x3d_enginestate_get_step();
        light.orientation.x = 0;
        
        x3d_lightmap_build(&light, context);
    }
    {
        X3D_SpotLight light;
        
        light.pos = x3d_vex3d_make(-150, -200, 400);
        light.orientation.y = 0;//x3d_enginestate_get_step();
        light.orientation.x = 256 - 64;
        
        x3d_lightmap_build(&light, context);
    }
    
    {
        X3D_SpotLight light;
        
        light.pos = x3d_vex3d_make(-500, 0, -200);
        light.orientation.y = 32;//x3d_enginestate_get_step();
        light.orientation.x = 0;
        
        x3d_lightmap_build(&light, context);
    }
}

void x3d_lightmap_bilinear_filter(X3D_LightMap* map) {
    uint8 data[map->h][map->w];
    
    int16 i, j;
    for(i = 0; i < map->h; ++i) {
        for(j = 0; j < map->w; ++j) {
            int16 count = 1;
            int16 sum = map->data[i * map->w + j];
            
            if(i > 0) {
                ++count;
                sum += map->data[map->w * (i - 1) + j];
            }
            
            if(j > 0) {
                ++count;
                sum += map->data[map->w * i + j - 1];
            }
            
            if(i + 1 < map->h) {
                ++count;
                sum += map->data[map->w * (i + 1) + j];
            }
            
            if(j + 1 < map->w) {
                ++count;
                sum += map->data[i * map->w + j + 1];
            }
            
            data[i][j] = sum / count;
        }
    }
    
    for(i = 0; i < map->h; ++i) {
        for(j = 0; j < map->w; ++j) {
            map->data[i * map->w + j] = data[i][j];
        }
    }
}

void test_lightmap(void) {
//     X3D_SpotLight light;
//     
//     X3D_LightMapContext context;
//     
//     light.pos = x3d_vex3d_make(0, 0, -200);
//     light.orientation.y = x3d_enginestate_get_step();
//     light.orientation.x = 0;
//     
//     x3d_lightmapcontext_init(&context, global_level);
//     x3d_lightmap_build(&light, &context);
//     x3d_lightmap_blit(context.maps + x3d_segfaceid_create(0, 0));
//     x3d_lightmapcontext_cleanup(&context);
}

