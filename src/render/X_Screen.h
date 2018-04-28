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

#include "X_Palette.h"
#include "math/X_fix.h"
#include "X_Texture.h"

#define X_ZBUF_FURTHEST_VALUE 0

struct X_Screen;
struct X_EngineContext;
struct X_CameraObject;

typedef struct X_ScreenEventHandlers
{
    void* userData;         // Pointer to user-defined data passed to screen updates
    void (*displayFrame)(struct X_Screen* screen, void* userData);
    void (*restartVideo)(struct X_EngineContext* context, void* userData);
    void (*cleanupVideo)(struct X_EngineContext* context, void* userData);
    bool (*isValidResolution)(int w, int h);
} X_ScreenEventHandlers;

typedef struct X_Screen
{
    X_Texture canvas;
    x_fp0x16* zbuf;
    
    struct X_CameraObject* cameraListHead;
    const X_Palette* palette;
    X_ScreenEventHandlers handlers;
} X_Screen;

void x_screen_attach_camera(X_Screen* screen, X_CameraObject* camera);
void x_screen_detach_camera(X_Screen* screen, X_CameraObject* camera);

void x_screen_restart_video(X_Screen* screen, int newW, int newH, x_fp16x16 newFov);

static inline size_t x_screen_zbuf_size(const X_Screen* screen)
{
    return screen->canvas.totalTexels() * sizeof(short);
}

static inline void x_screen_init(X_Screen* screen, int w, int h, X_ScreenEventHandlers* handlers)
{
    screen->canvas.resize(w, h);
    screen->zbuf = (x_fp0x16*)x_malloc(x_screen_zbuf_size(screen));
    
    screen->cameraListHead = NULL;
    screen->handlers = *handlers;
}

static inline void x_screen_set_palette(X_Screen* screen, const X_Palette* palette)
{
    screen->palette = palette;
}

static inline void x_screen_cleanup(X_Screen* screen)
{
    x_free(screen->zbuf);
    
    /// @todo Should we detach all of the attached cameras?
}

static inline int x_screen_w(const X_Screen* screen)
{
    return screen->canvas.getW();
}

static inline int x_screen_h(const X_Screen* screen)
{
    return screen->canvas.getH();
}

static inline X_Vec2 x_screen_center(const X_Screen* screen)
{
    return x_vec2_make(x_screen_w(screen) / 2, x_screen_h(screen) / 2);
}

static inline void x_screen_zbuf_clear(X_Screen* screen)
{
    memset(screen->zbuf, X_ZBUF_FURTHEST_VALUE, x_screen_zbuf_size(screen));
}

