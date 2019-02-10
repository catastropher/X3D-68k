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

#include "Player.hpp"

fp g_gamma = fp::fromInt(1);

void cmdGamma(X_EngineContext* engineContext, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_printf(engineContext->getConsole(), "Gamma is currently %f\n", g_gamma.toFloat());

        return;
    }

    g_gamma = fp::fromFloat(atof(argv[1]));

    X_Palette* newPalette = new X_Palette;

    *newPalette = *x_palette_get_quake_palette();
    x_palette_correct_gamma(newPalette, g_gamma);

    Engine::getInstance()->getScreen()->palette = newPalette;
}

static Entity* createEntityCallback(const char* entityType, X_Edict& edict, BspLevel& level)
{
    if(strcmp(entityType, "info_player_start") == 0)
    {
        return new Player(edict, level);
    }

    return nullptr;
}

int main(int argc, char* argv[])
{
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
        .defaultFont("../assets/font.xtex")
        .screenConfig(screenConfig);

    config.systemConfig.programPath = argv[0];

    X_EngineContext* engineContext = Engine::init(config);

    FileSystem::addSearchPath("../assets");
    FileSystem::addSearchPath("../maps");

    engineContext->entityManager->setCreateEntityCallback(createEntityCallback);

    x_console_execute_cmd(engineContext->getConsole(), "exec engine.cfg;exec ../engine.cfg");

    x_console_register_cmd(engineContext->getConsole(), "screen.gamma", cmdGamma);

    Engine::run();
}

