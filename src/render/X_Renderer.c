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

#include "X_Renderer.h"

static void x_renderer_init_console_vars(X_Renderer* renderer, X_Console* console)
{
    x_console_register_var(console, &renderer->varFillColor, &renderer->fillColor, "render.fillColor", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->varShowFps, &renderer->showFps, "render.showFps", X_CONSOLEVAR_BOOL, "0", 0);
}

void x_renderer_init(X_Renderer* renderer, X_Console* console, X_Screen* screen)
{
    x_renderer_init_console_vars(renderer, console);
    x_ae_context_init(&renderer->activeEdgeContext, screen, 5000, 5000, 5000);
}

