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

Vec3fp position = Vec3fp(0, 0, 0);
Mat4x4 orientation;

TestGame* game;

void customRenderCallback(X_EngineContext* engineContext, X_RenderContext* renderContext)
{

 //   polygon.renderWireframe(*renderContext, 255);
    
  //  engineContext->getRenderer()->activeEdgeContext.addPortalPolygon(polygon, tracer.collisionPlane, BoundBoxFrustumFlags((1 << 4) - 1), 0);
}

int main(int argc, char* argv[])
{
    // int val = 581.641052 * 65536;
    // int orig = val;

    // int shift = 0;

    // while(val >= 65536)
    // {
    //     val >>= 1;
    //     ++shift;
    // }

    // printf("Shifted down: %d, shift = %d\n", val, shift);

    // int real = (1.0 / orig) * 65536 * 65536;

    // printf("Answer: %d\n", real);

    // int shiftUp;
    // int r = x_fastrecip_unshift(val, shiftUp);

    // printf("Shift up: %d\n", shiftUp);

    // printf("R: %d\n", r >> shift);


    Vec3Template<fp> v;
    Quaternion q1 = Quaternion::fromAxisAngle(v, fp(0));

    auto q2 = q1 * q1;

    SystemConfig sysConfig;

    sysConfig.programPath = argv[0];

    initSystem(sysConfig);

    Log::info("Hello world!!!!!!!!!!!!!!\n");

    // int size;
    // char* data = FileReader::readWholeFile("../settings.json", size);

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

