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

#include "X_EngineContext.h"
#include "error/X_error.h"

static inline void init_object_factory(X_EngineContext* context)
{
    x_factory_init(&context->gameObjectFactory, 5, 10);
}

static inline void init_screen(X_EngineContext* context, int screenW, int screenH)
{
    x_screen_init(&context->screen, screenW, screenH);
}

static inline void init_main_font(X_EngineContext* context, const char* fontFileName, int fontW, int fontH)
{
    _Bool fontLoaded = x_font_load_from_xtex_file(&context->mainFont, fontFileName, fontW, fontH);
    x_assert(fontLoaded, "Failed to load main font");
}

static inline void init_console(X_EngineContext* context)
{
    x_console_init(&context->console, &context->screen, &context->mainFont);
    x_console_print(&context->console, "Console initialized.\n");
}

static inline void init_keystate(X_EngineContext* context)
{
    x_keystate_init(&context->keystate);
}

static inline void cleanup_object_factory(X_EngineContext* context)
{
    x_factory_cleanup(&context->gameObjectFactory);
}

static inline void cleanup_screen(X_EngineContext* context)
{
    x_screen_cleanup(&context->screen);
}

static inline void cleanup_main_font(X_EngineContext* context)
{
    x_font_cleanup(&context->mainFont);
}

static inline void cleanup_console(X_EngineContext* context)
{
    x_console_cleanup(&context->console);
}

////////////////////////////////////////////////////////////////////////////////
/// Initializes an engine context.
///
/// @param context  - context to initialize
/// @param screenW  - width of the screen
/// @param screenH  - height of the screen
///
/// @note Make sure to call @ref x_enginecontext_cleanup() when done to release
///     the allocated resources!
////////////////////////////////////////////////////////////////////////////////
void x_enginecontext_init(X_EngineContext* context, int screenW, int screenH)
{
    init_object_factory(context);
    init_screen(context, screenW, screenH);
    init_main_font(context, "font.xtex", 8, 8);
    init_console(context);
    init_keystate(context);
}

////////////////////////////////////////////////////////////////////////////////
/// Cleans up an engine context.
////////////////////////////////////////////////////////////////////////////////
void x_enginecontext_cleanup(X_EngineContext* context)
{
    cleanup_object_factory(context);
    cleanup_screen(context);
    cleanup_main_font(context);
    cleanup_console(context);
}


