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

#include "X_Canvas.h"
#include "object/X_CameraObject.h"
#include "X_Palette.h"

typedef struct X_Screen
{
    X_Canvas canvas;
    X_CameraObject* cameraListHead;
    const X_Palette* palette;
} X_Screen;

void x_screen_attach_camera(X_Screen* screen, X_CameraObject* camera);
void x_screen_detach_camera(X_Screen* screen, X_CameraObject* camera);

static inline void x_screen_init(X_Screen* screen, int w, int h)
{
    x_canvas_init(&screen->canvas, w, h);
    screen->cameraListHead = NULL;
}

static inline void x_screen_set_palette(X_Screen* screen, const X_Palette* palette)
{
    screen->palette = palette;
}

static inline void x_screen_cleanup(X_Screen* screen)
{
    x_canvas_cleanup(&screen->canvas);
    /// @todo Should we detach all of the attached cameras?
}

static inline int x_screen_w(const X_Screen* screen)
{
    return x_canvas_w(&screen->canvas);
}

static inline int x_screen_h(const X_Screen* screen)
{
    return x_canvas_h(&screen->canvas);
}

