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

#pragma once

#include "X_EngineContext.h"

#define X_MAJOR_VERSION 0
#define X_MINOR_VERSION 1
#define X_VERSION (X_MAJOR_VERSION * 1000 + X_MINOR_VERSION)

X_EngineContext* x_engine_init(X_Config* config);
void x_engine_cleanup(void);
void x_engine_render_frame(X_EngineContext* engineContext);
void x_engine_update_objects(X_EngineContext* engineContext);

X_Console* x_engine_get_console(void);

struct Config;

void initSystem(SystemConfig& config);

static inline bool x_engine_level_is_loaded(const X_EngineContext* context)
{
    return x_bsplevel_file_is_loaded(context->getCurrentLevel());
}

static inline X_BspLevel* x_engine_get_current_level(X_EngineContext* context)
{
    return context->getCurrentLevel();
}

