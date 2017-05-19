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

static inline void init_object_factory(X_EngineContext* context)
{
    x_factory_init(&context->gameObjectFactory, 5, 10);
}

static inline void init_screen(X_EngineContext* context, int screenW, int screenH)
{
    x_screen_init(&context->screen, screenW, screenH);
}

static inline void cleanup_object_factory(X_EngineContext* context)
{
    x_factory_cleanup(&context->gameObjectFactory);
}

void x_enginecontext_init(X_EngineContext* context, int screenW, int screenH)
{
    init_object_factory(context);
    init_screen(context, screenW, screenH);
}

void x_enginecontext_cleanup(X_EngineContext* context)
{
    cleanup_object_factory(context);
}


