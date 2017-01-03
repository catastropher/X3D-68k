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
//
//
//
// test.c -> test for PC

#include <X3D/X3D.h>

#if defined(__pc__)
    #include <SDL2/SDL.h>
    #include <alloca.h>
#endif

#if defined(__nspire__)
    #include <SDL/SDL.h>
    #include <alloca.h>
#endif


void engine_test_handle_keys(void);
void setup_key_map(void);

extern X3D_Level* global_level;

void setup_camera(void) {
  x3d_camera_init();
  
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;

  X3D_Prism3D prism = { .v = alloca(1000) };
  X3D_LevelSegment* seg = x3d_level_get_segmentptr(global_level, 0);
  x3d_levelsegment_get_geometry(global_level, seg, &prism);
  
  X3D_Vex3D center = { 0, 0, 0 };
  x3d_prism3d_center(&prism, &center);
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { (int32)center.x << 8, (int32)center.y << 8, (int32)center.z << 8 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
}

void segment_face_render_callback(X3D_SegmentRenderFace* face);


X3D_Texture checkerboard;
X3D_Texture checkerboard2;

void test_lightmap(void);

X3D_Model teapot;

void test_render_callback(X3D_CameraObject* cam) {    
    //X3D_ColorIndex red = x3d_color_to_colorindex(x3d_rgb_to_color(255, 0, 0));
    //x3d_model_render_wireframe(&teapot, cam, red);
}

X3D_LightMapContext lightmap_context;

void add_test_lights(X3D_LightMapContext* context);

void build_test_level(void) {
    static X3D_Level level;
    
    x3d_level_init(&level);
    
    X3D_Prism3D prism = { .v = alloca(1000) };
    x3d_prism3d_construct(&prism, 8, 16 * X3D_UNITS_PER_FOOT, 8 * X3D_UNITS_PER_FOOT, (X3D_Vex3D_angle256) { 0, ANG_45, 0 });
    uint16 seg0 = x3d_level_add_new_standalone_segment(&level, &prism, 0)->id;
    
    
#if 0
    uint16 seg2 = x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(seg0, 1), 400)->id;
    
    x3d_levelsegment_get_geometry(&level, x3d_level_get_segmentptr(&level, seg2), &prism);
    
    X3D_Polygon3D* poly = x3d_polygon3d_temp();
    x3d_prism3d_get_face(&prism, 1, poly);
    X3D_Vex3D shift = { 0, -200, 0 };
    x3d_polygon3d_scale(poly, 128);
    x3d_prism3d_set_face(&prism, 1, poly);
    
    //x3d_levelsegment_update_geometry(&level, x3d_level_get_segmentptr(&level, seg), &prism);
    
    uint16 seg3 = x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(0, 3), 200)->id;
    uint16 seg1 = x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(seg3, 1), 800)->id;
    
    x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(seg1, 2), 800);
    x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(seg1, 1), 800);
    x3d_level_add_extruded_segment(&level, x3d_segfaceid_create(seg1, 3), 800);
    
    X3D_LevelSegment* seg1ptr = x3d_level_get_segmentptr(&level, seg1);
    
    
    uint16 add_face = 5;
    uint16 new_seg = x3d_level_add_wall_segment_to_center_of_face(&level, x3d_level_get_segmentptr(&level, seg0), add_face, 4, 100, 100)->id;
    
    uint16 id = x3d_segfaceid_create(new_seg, 0);
    
    x3d_levelsegment_set_wall_segs_for_face(&level, x3d_level_get_segmentptr(&level, seg0), add_face, 
        &id, 1
    );
#endif
    global_level = &level;
   
#if 0
    if(!x3d_level_load(&level, "/home/michael/code/XBuilder/build/test.xlev")) {
        x3d_log(X3D_ERROR, "Failed to load level");
        exit(0);
    }
#endif
    
    
    x3d_lightmapcontext_init(&lightmap_context, &level);
    
    uint16 i;
    for(i = 0; i < lightmap_context.total_lightmaps; ++i) {
        if(lightmap_context.lightmaps[i].texels) {
            //x3d_lightmap_bilinear_filter(lightmap_context.lightmaps + i);
        }
    }
    
    
    x3d_lightmapcontext_build_surfaces(&lightmap_context, &checkerboard);
    //x3d_build_combined_lightmap_texture();
}

void init() {
#if defined(__pc__) && 1
    int16 w = 640;
    int16 h = 480;
#else
    int16 w = 320;
    int16 h = 240;
#endif
  
    X3D_InitSettings init = {
        .screen_w = w,
        .screen_h = h,
        .screen_scale = 1,
        .fullscreen = X3D_FALSE,
        .fov = ANG_60
    };

    x3d_init(&init);
}

void test_mat4x4();

#include <X3D/file/X3D_resourcepack.h>

void test_packfile() {
    char data[] = "Hello world!";
    
    X3D_ResourcePackFile files[2] = {
        {
            .name = "tex/floor.xtex",
            .size = sizeof(data),
            .data_offset = 0,
            .loaded_data = data
        },
        {
            .name = "light/maps.tex",
            .size = sizeof(data),
            .data_offset = 0,
            .loaded_data = data
        }
    };
    
    x3d_resourcepack_save_packfiles_to_file(files, 2, "test.pak");
    
    X3D_ResourcePack pack;
    x3d_resourcepack_load_from_file(&pack, "test.pak");
    x3d_resourcepack_print_file_header(&pack);
    
    X3D_Buffer buf;
    x3d_resourcepack_open_packfile(&pack, "light/maps.tex", &buf);
    
    printf("Data in file: '%s'\n", buf.data);
    
    x3d_resourcepack_close_packfile(&pack, &buf);
}

int main(int argc, char* argv[]) {
   // test_packfile();
   // return 0;
    
    
    init();
    
    //test_mat4x4();
    
    x3d_model_load_from_file(&teapot, "/home/michael/code/X3D-68k/tools/build/teapot.xmod");
    
    
    
    //x3d_texture_from_array(&checkerboard, wood_tex_data);
    if(!x3d_texture_load_from_bmp_file(&checkerboard, "floor.bmp")) {
        x3d_log(X3D_ERROR, "Failed to load texture");
        return 0;
    }
    //x3d_texture_fill_with_checkerboard(&checkerboard, 64, 4);
    
    x3d_rendermanager_set_render_mode(X3D_RENDER_WIREFRAME);
    
    //x3d_texture_load_from_file(&checkerboard2, "walrii.bmp");
    
    x3d_texture_init(&checkerboard2, 128, 128, 0);
    //x3d_texture_create_new(&checkerboard2, 128, 128, x3d_rgb_to_color(0, 0, 255));
    
    x3d_rendermanager_get()->near_z = 10;
    
    build_test_level();
    
    setup_key_map();
    x3d_keymanager_set_callback(engine_test_handle_keys);
    setup_camera();
    
    x3d_game_loop();

    x3d_cleanup();  
    
    return 0;
}
