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

#include "game.hpp"

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
    
    //x_console_execute_cmd(&context->engineContext->console, "map e1m1");
    //x_gameobjectloader_load_objects(context->engineContext, context->engineContext->currentLevel.entityDictionary);
    
    while(!context->quit)
    {   
        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(context->engineContext, context->cam, &renderContext);
        
        render(context);

        handle_keys(context);
        screen_update(context);
    }    
}

int main(int argc, char* argv[])
{
    const char* programPath = argv[0];

    int screenW = 640;
    int screenH = 480;

    ScreenConfig screenConfig = ScreenConfig()
        .fieldOfView(X_ANG_60)
        .resolution(screenW, screenH)
        .useQuakeColorPalette();

    //screen_set_callbacks(context, screenConfig);

    X_Config config = X_Config()
        .programPath(programPath)
        .defaultFont("font.xtex")
        .screenConfig(screenConfig);

    TestGame game(config);
    game.run();
}

