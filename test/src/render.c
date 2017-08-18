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

#include "Context.h"

static void draw_fps(X_EngineContext* context)
{
    int diff = context->frameStart - context->lastFrameStart;
    int fps;
    
    if(diff == 0)
        fps = 1000;
    else
        fps = 1000 / diff;
    
    char fpsStr[20];
    sprintf(fpsStr, "%d", fps);
    
    X_Vec2 pos = x_vec2_make(x_screen_w(&context->screen) - x_font_str_width(&context->mainFont, fpsStr), 0);
    x_canvas_draw_str(&context->screen.canvas, fpsStr, &context->mainFont, pos);
}

static void draw_current_leaf_info(X_CameraObject* cam, X_RenderContext* renderContext)
{
    char str[128];
    sprintf(str, "Current Leaf: %d\nVisible leaves: %d\n", (int)(cam->currentLeaf - renderContext->level->leaves),
            x_bsplevel_count_visible_leaves(renderContext->level, cam->pvsForCurrentLeaf));
    
    x_canvas_draw_str(renderContext->canvas, str, &renderContext->engineContext->mainFont, x_vec2_make(0, 0));
}

static void draw_crosshair(X_EngineContext* engineContext)
{
    X_Color white = engineContext->screen.palette->white;
    X_Texture* tex = &engineContext->screen.canvas.tex;
    
    int centerX = tex->w / 2;
    int centerY = tex->h / 2;
    
    x_texture_set_texel(tex, centerX - 1, centerY, white);
    x_texture_set_texel(tex, centerX + 1, centerY, white);
    x_texture_set_texel(tex, centerX, centerY - 1, white);
    x_texture_set_texel(tex, centerX, centerY + 1, white);
}

void render(Context* context)
{
    X_EngineContext* engineContext = context->engineContext;
    
    x_engine_render_frame(engineContext);
    
    if(x_console_is_open(&engineContext->console))
        x_console_render(&engineContext->console);
}


