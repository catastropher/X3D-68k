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

#include <X3D/X3D.hpp>
#include <SDL/SDL.h>
#include <unistd.h>
#include <math.h>

#include "Context.h"
#include "screen.h"
#include "keys.h"
#include "init.h"
#include "render.h"

#include "game.hpp"

TestGame* game;

void customRenderCallback(X_EngineContext* engineContext, X_RenderContext* renderContext)
{

 //   polygon.renderWireframe(*renderContext, 255);
    
  //  engineContext->getRenderer()->activeEdgeContext.addPortalPolygon(polygon, tracer.collisionPlane, BoundBoxFrustumFlags((1 << 4) - 1), 0);
}

int main(int argc, char* argv[])
{
    SystemConfig sysConfig;

    sysConfig.programPath = argv[0];

    initSystem(sysConfig);

    FileSystem::addSearchPath("../assets");
    FileSystem::addSearchPath("../maps");

#ifdef __nspire__
    int screenW = 320;
    int screenH = 240;
#else
    int screenW = 640;
    int screenH = 480;
#endif

    ScreenConfig screenConfig = ScreenConfig()
        .fieldOfView(X_ANG_60)
        .resolution(screenW, screenH)
        .useQuakeColorPalette();
    
    X_Config config = X_Config()
        .programPath(argv[0])
        .defaultFont("font.xtex")
        .screenConfig(screenConfig);

    TestGame game(config);
    ::game = &game;
    game.run();

    MemoryManager::cleanup();

    x_log_cleanup();
}

