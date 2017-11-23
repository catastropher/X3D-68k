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

X_Color tintTable[256];

void cmd_tint(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 5)
        return;
    
    const X_Palette* palette = x_palette_get_quake_palette();
    unsigned char r = atoi(argv[1]);
    unsigned char g = atoi(argv[2]);
    unsigned char b = atoi(argv[3]);
    
    
    float alpha = atof(argv[4]);
    
    for(int i = 0; i < 256; ++i)
    {
        unsigned char rr, gg, bb;
        x_palette_get_rgb(palette, i, &rr, &gg, &bb);
        
        unsigned newR = alpha * r + (1 - alpha) * rr;
        unsigned newG = alpha * g + (1 - alpha) * gg;
        unsigned newB = alpha * b + (1 - alpha) * bb;
        
        tintTable[i] = x_palette_get_closest_color_from_rgb(palette, newR, newG, newB);
    }
}

X_Vec3 quad[4];
X_Cube cube;

void build_quad(X_Vec3_fp16x16 center)
{
    x_cube_init(&cube, 50, 50, 50);
    x_cube_translate(&cube, center);
    
    X_Polygon3 p = x_polygon3_make(quad, 4);
    x_cube_get_face(&cube, 2, &p);
    x_polygon3_fp16x16_to_polygon3(&p, &p);
}

void draw_quad(X_RenderContext* renderContext)
{
    X_Polygon3 p = x_polygon3_make(quad, 3);
    x_polygon3_render_transparent(&p, renderContext, tintTable);
    
    X_Vec3 v[3] = { quad[2], quad[3], quad[0] };
    p.vertices = v;
    
    
    x_polygon3_render_transparent(&p, renderContext, tintTable);
    
    x_cube_render(&cube, renderContext, 255);
}

void gameloop(Context* context)
{
   // X_EntityModel model;
   // x_entitymodel_load_from_file(&model, "shambler.mdl");
    
    int frameId = 0;
    int count = 0;
    
    x_console_register_cmd(&context->engineContext->console, "tint", cmd_tint);
    
    X_EntityFrame* frame = NULL;
    
    for(int i = 0; i < 256; ++i)
        tintTable[i] = i;
    
    build_quad(x_vec3_origin());
    
    while(!context->quit)
    {
        render(context);
        
//         for(int i = 0; i < model.totalSkins; ++i)
//         {
//             X_Texture skinTex;
//             x_entitymodel_get_skin_texture(&model, 0, 0, &skinTex);
//             x_canvas_blit_texture(&context->engineContext->screen.canvas, &skinTex, x_vec2_make(0, i * model.skinHeight));
//         }
        
        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(context->engineContext, context->cam, &renderContext);

        X_Vec3 v[3] = 
        {
            x_vec3_make(0, 0, 0),
            x_vec3_make(200, 0, 0),
            x_vec3_make(200, -200, 0),
        };
        
        if(x_keystate_key_down(&context->engineContext->keystate, 't'))
            build_quad(x_cameraobject_get_position(context->cam));
        
        draw_quad(&renderContext);
        
        
        if(++count == 10)
        {
            count = 0;
            //frame = frame->nextInSequence;
        }
        
//         if(!frame)
//             frame = x_entitymodel_get_animation_start_frame(&model, "swingr");
//         
//         if(!frame)
//             x_system_error("No such frame");
        
        //x_entitymodel_draw_frame_wireframe(&model, frame, x_vec3_make(0, 0, 0), 255, &renderContext);
        //x_entitymodel_render_flat_shaded(&model, frame, &renderContext);
        
        
        handle_keys(context);
        
        X_Texture* tex = &context->engineContext->screen.canvas.tex;
        
        //for(int i = 0; i < x_texture_total_texels(tex); ++i)
        //    tex->texels[i] = tintTable[tex->texels[i]];
        
        screen_update(context);
    }
    
    //x_entitymodel_cleanup(&model);
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, argv[0]);    
    gameloop(&context);
    cleanup(&context);
}

