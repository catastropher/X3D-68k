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

#include "engine/EngineContext.hpp"
#include "OverlayRenderer.hpp"
#include "dev/console/Console.hpp"

OverlayRenderer::OverlayRenderer(Console& console)
    : isEnabled(false)
{
    x_console_register_cmd(&console, "overlay", cmdOverlay);
}

void OverlayRenderer::render()
{
    if(!isEnabled)
    {
        return;
    }

    for(Overlay* overlay : overlays)
    {
        overlay->render();
    }
}

bool OverlayRenderer::setOverlayIsEnabled(const char* overlayName, bool isEnabled, int argc, char* argv[])
{
    for(Overlay* overlay : overlays)
    {
        if(strcmp(overlayName, overlay->name) == 0)
        {
            overlay->isEnabled = isEnabled;

            if(isEnabled)
            {
                overlay->enable(argc, argv);
            }

            return true;
        }
    }

    return false;
}

void OverlayRenderer::cmdOverlay(EngineContext* engineContext, int argc, char** argv)
{
    Console* console = engineContext->console;
    OverlayRenderer* overlayRenderer = engineContext->overlayRenderer;

    // TODO: print usage
    if(argc == 2)
    {
        bool isEnabled = atoi(argv[1]);
        overlayRenderer->setIsEnabled(isEnabled);

        x_console_printf(
            console,
            "Overlay %s\n",
            isEnabled
                ? "enabled"
                : "disabled");

        return;
    }
    else if(argc == 3)
    {
        const char* overlayName = argv[1];
        bool overlayIsEnabled = atoi(argv[2]);

        if(overlayRenderer->setOverlayIsEnabled(overlayName, overlayIsEnabled, argc - 3, argv + 3))
        {
            x_console_printf(
                console,
                "Overlay %s %s\n",
                overlayName,
                overlayIsEnabled
                ? "enabled"
                : "disabled");
        }
        else
        {
            x_console_print(console, "No such overlay");
        }
    }
    else
    {
        x_console_print(console, "Wrong number of arguments\n");
    }
}
