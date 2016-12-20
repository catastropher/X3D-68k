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

#include "X3D_polygon.h"
#include "X3D_camera.h"
#include "X3D_render.h"
#include "X3D_enginestate.h"
#include "render/X3D_lightmap.h"
#include "render/X3D_texture.h"
#include "render/geo/X3D_render_polygon.h"

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
        .render_mode = X3D_RENDER_ID_BUFFER
    };
    
    x3d_screen_zbuf_clear();
    x3d_render(light_cam, &render_context);
    
    X3D_Vex3D v;
    x3d_calculate_position_from_z_buffer(640 / 2, 480 / 2, light_cam, &v);    
}

extern X3D_Level* global_level;

void x3d_lightmap_init(X3D_LightMapContext* context, uint32 id, X3D_Polygon3D* poly) {
    X3D_PlanarProjection* proj = context->proj + id;
    x3d_planarprojection_build_from_polygon3d(proj, poly);
    
    X3D_Texture* map = context->lightmaps + id;
    uint16 w = x3d_units_to_texels(proj->max_x - proj->min_x) + 1;
    uint16 h = x3d_units_to_texels(proj->max_y - proj->min_y) + 1;
    
    x3d_texture_init(map, w, h, 0);
    x3d_texture_fill(map, 0);
}

void x3d_lightmapcontext_init(X3D_LightMapContext* context, X3D_Level* level) {
    context->total_lightmaps = x3d_segfaceid_create(level->segs.total, 0);
    
    context->lightmaps = malloc(sizeof(X3D_Texture) * context->total_lightmaps);
    context->surfaces = malloc(sizeof(X3D_Texture) * context->total_lightmaps);
    context->proj = malloc(sizeof(X3D_PlanarProjection) * context->total_lightmaps);
    
    context->total_light = 0;
    context->level = level;
    
    X3D_Prism3D prism = { .v = alloca(1000) };
    X3D_Polygon3D poly = { .v = alloca(1000) };
    
    
    uint32 i, j;
    for(i = 0; i < context->total_lightmaps; ++i) {
        x3d_texture_init_empty(context->lightmaps + i);
        x3d_texture_init_empty(context->surfaces + i);
    }
    
    
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


void x3d_lightmapcontext_apply_light(X3D_LightMapContext* context, uint16 id, X3D_Vex2D pos, uint8 value) {
    X3D_Texture* map = context->lightmaps + id;
    uint32 current_value = x3d_texture_get_texel(map, pos.x, pos.y);

    x3d_texture_set_texel(map, pos.x, pos.y, X3D_MIN(255, current_value + value));
}

extern float* id_zbuf;

void x3d_lightmap_build(X3D_SpotLight* light, X3D_LightMapContext* context) {
    x3d_log(X3D_INFO, "Baking light map...");
    
    int32 i, j, k;
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_CameraObject light_cam;
    
    id_zbuf = malloc(sizeof(float) * screenman->w * screenman->h);
    
    for(i = 0; i < (uint32)screenman->w * screenman->h; ++i) {
        id_zbuf[i] = 0x7F7F;
    }
    
    _Bool render_surface[context->total_lightmaps];
    
    for(i = 0; i < context->total_lightmaps; ++i)
        render_surface[i] = X3D_FALSE;
    
    x3d_render_from_perspective_of_spotlight(light, &light_cam);
        
    for(i = 0; i < screenman->h; ++i) {
        for(j = 0; j < screenman->w; ++j) {
            uint32 index = (uint32)x3d_screen_get_internal_value(j, i);
            
            if(index < context->total_lightmaps) {                
                render_surface[index] = X3D_TRUE;
            }
        }
    }
    
    for(i = 0; i < context->total_lightmaps; ++i) {
        if(!x3d_texture_is_empty(context->lightmaps + i) && render_surface[i]) {
            x3d_log(X3D_INFO, "Surface %d", i);
            
            X3D_Texture* map = context->lightmaps + i;
            
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
                        
                        if((transformed.z > 0 && transformed.z < z + 50) || x3d_screen_get_internal_value(k, j) == i) {
                            float dist = sqrt(pow(projected.x - screenman->w / 2, 2) + pow(projected.y - screenman->h / 2, 2));
                            
                            if(dist < 3 * 150 / 2) {
                                //printf("Transformed: %d -> %d\n", transformed.z, z);
                                x3d_lightmapcontext_apply_light(context, i, v2d, 64 * (6000 - transformed.z) / 6000);
                            }
                        }
                    }
                }
            }
        }
    }
    
    free(id_zbuf);
    id_zbuf = NULL;
}

void x3d_lightmapcontext_cleanup(X3D_LightMapContext* context) {
    uint32 i;
    for(i = 0; i < context->total_lightmaps; ++i) {
        x3d_texture_cleanup(context->lightmaps);
        x3d_texture_cleanup(context->surfaces);
    }
    
    free(context->lightmaps);
    free(context->surfaces);
    free(context->proj);
}

void x3d_lightmap_blit(X3D_Texture* map) {
    uint32 i, j;

    if(x3d_texture_is_empty(map))
        return;
    
    for(i = 0; i < map->h; ++i) {
        for(j = 0; j < map->w; ++j) {
            uint8 val = x3d_texture_get_texel(map, j, i);
            x3d_screen_draw_pix(j, i, x3d_rgb_to_color(val, val, val));
        }
    }
}

void x3d_texture_apply_lightmap(X3D_Texture* tex, X3D_Texture* map) {
    int32 i;
    for(i = 0; i < x3d_texture_total_texels(tex); ++i) {
        tex->texels[i] = x3d_colormap_get_index(tex->texels[i], map->texels[i] / 16);
    }
}

void x3d_build_combined_lightmap_texture(X3D_Texture* tex, X3D_Texture* map, X3D_Polygon3D* poly, X3D_Texture* dest) {
    X3D_PlanarProjection proj;
    x3d_planarprojection_build_from_polygon3d(&proj, poly);
    
    X3D_RasterPolygon2D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        X3D_Vex2D v;
        x3d_planarprojection_project_point(&proj, poly->v + i, &v);
        rpoly.v[i].v.x = v.x;
        rpoly.v[i].v.y = v.y;
        
        rpoly.v[i].uu = v.x;
        rpoly.v[i].vv = v.y;
        rpoly.v[i].zz = 100;
    }
    
    X3D_PolygonRasterAtt att = {
        .surface = {
            .tex = tex
        }
    };

    int16 w = map->w;
    int16 h = map->h;
    
    int16* zbuf = x3d_zbuf_alloc(w, h);
    x3d_zbuf_clear(zbuf, w, h);
    
    x3d_log(X3D_INFO, "Surface size: %dx%d", w, h);
    
    x3d_texture_init(dest, w, h, 0);
    x3d_polygonrasteratt_set_screen_to_texture(&att, dest, zbuf);
    x3d_polygon2d_render_texture_surface(rpoly.v, rpoly.total_v, &att);
    
    x3d_texture_apply_lightmap(dest, map);
}

void x3d_lightmapcontext_build_surfaces(X3D_LightMapContext* context, X3D_Texture* level_tex) {
    uint16 i;
    X3D_Prism3D prism = { .v = alloca(1000) };
    X3D_Polygon3D poly = { .v = alloca(1000) };
    
    
    for(i = 0; i < context->total_lightmaps; ++i) {
        if(!x3d_texture_is_empty(context->lightmaps + i)) {
            if(!x3d_texture_is_empty(context->surfaces + i))
                x3d_texture_cleanup(context->surfaces + i);
            
            
            uint16 segid = x3d_segfaceid_seg(i);
            uint16 face = x3d_segfaceid_face(i);
            
            X3D_LevelSegment* seg = x3d_level_get_segmentptr(context->level, segid);
            x3d_levelsegment_get_geometry(context->level, seg, &prism);
            x3d_prism3d_get_face(&prism, face, &poly);
            
            x3d_build_combined_lightmap_texture(level_tex, context->lightmaps + i, &poly, context->surfaces + i);
        }
    }
}

void add_test_lights(X3D_LightMapContext* context) {
}

static inline uint8 x3d_lightmap_bilinear_filter_texel(const X3D_Texture* map, int16 x, int16 y) {
    const int16 offset[5][2] = {
        { 0, 0 },
        { 0, 1 },
        { 0, -1 },
        { 1, -1 },
        { 1, 1 }
    };
    
    int16 total = 0;
    int16 sum = 0;
    
    int16 k;
    for(k = 0; k < 5; ++k) {
        int16 u = x + offset[k][0];
        int16 v = y + offset[k][1];
        
        if(x3d_texture_texel_is_valid(map, u, v)) {
            sum += x3d_texture_get_texel(map, u, v);
            ++total;
        }
    }
    
    return sum / total;
}

void x3d_lightmap_bilinear_filter(X3D_Texture* map) {
    X3D_Texture output;
    x3d_texture_init(&output, map->w, map->h, 0);
    
    int16 i, j;
    for(i = 0; i < map->h; ++i) {
        for(j = 0; j < map->w; ++j) {
            uint8 value = x3d_lightmap_bilinear_filter_texel(map, j, i);
            x3d_texture_set_texel(&output, j, i, value);
        }
    }
    
    x3d_texture_copy_texels(map, &output);
    x3d_texture_cleanup(&output);
}

void test_lightmap(void) {
}

