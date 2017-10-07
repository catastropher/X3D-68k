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

void gameloop(Context* context)
{
    X_EntityModel model;
    x_entitymodel_load_from_file(&model, "ogre.mdl");
    
    int frameId = 0;
    int count = 0;
    
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
        
        if(++count == 10)
        {
            count = 0;
            frameId = (frameId + 1) % 8;
        }
        
        char name[16];
        sprintf(name, "run%d", frameId + 1);
        
        X_EntityFrame* frame = x_entitymodel_get_frame(&model, name);
        
        if(!frame)
            x_system_error("No such frame %s", name);
        
        x_entitymodel_draw_frame_wireframe(&model, frame, x_vec3_make(0, 0, 0), 255, &renderContext);
        
        handle_keys(context);        
        screen_update(context);
    }
    
    x_entitymodel_cleanup(&model);
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, argv[0]);    
    gameloop(&context);
    cleanup(&context);
}

