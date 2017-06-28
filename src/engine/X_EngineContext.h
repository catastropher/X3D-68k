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

#include "memory/X_Factory.h"
#include "render/X_Screen.h"
#include "dev/X_Console.h"
#include "render/X_Font.h"

////////////////////////////////////////////////////////////////////////////////
/// A context object that holds the state for the entire engine.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_EngineContext
{
    X_Factory gameObjectFactory;        ///< Factory for creating new game objects
    X_Screen screen;                    ///< Virtual screen
    X_Console console;
    X_Font mainFont;
    X_KeyState keystate;
} X_EngineContext;

void x_enginecontext_init(X_EngineContext* context, int screenW, int screenH);
void x_enginecontext_cleanup(X_EngineContext* context);

