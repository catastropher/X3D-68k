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

class X_Screen;
struct X_EngineContext;
struct X_CameraObject;

struct X_Font;

typedef struct X_ScreenEventHandlers
{
    void* userData;         // Pointer to user-defined data passed to screen updates
    void (*displayFrame)(struct X_Screen* screen, void* userData);
    void (*restartVideo)(struct X_EngineContext* context, void* userData);
    void (*cleanupVideo)(struct X_EngineContext* context, void* userData);
    bool (*isValidResolution)(int w, int h);
} X_ScreenEventHandlers;



class X_Screen
{
public:
    X_Screen(int w, int h, X_ScreenEventHandlers& handlers)
    {
        canvas.resize(w, h);
        zbuf = (x_fp0x16*)x_malloc(calculateZBufSize());
        
        cameraListHead = NULL;
        this->handlers = handlers;
    }
    
    void setPalette(const X_Palette* palette)
    {
        this->palette = palette;
    }
    
    int getW() const
    {
        return canvas.getW();
    }
    
    int getH() const
    {
        return canvas.getH();
    }
    
    void clearZBuf()
    {
        memset(zbuf, X_ZBUF_FURTHEST_VALUE, calculateZBufSize());
    }
    
    X_Vec2 getCenter()
    {
        return x_vec2_make(getW() / 2, getH() / 2);
    }
    
    void attachCamera(X_CameraObject* camera);
    void detachCamera(X_CameraObject* camera);
    void restartVideo(int newW, int newH, x_fp16x16 newFov);
    
    ~X_Screen()
    {
        x_free(zbuf);
    }
    
    X_Texture canvas;
    x_fp0x16* zbuf;
    
    struct X_CameraObject* cameraListHead;
    const X_Palette* palette;
    X_ScreenEventHandlers handlers;
    
private:
    int calculateZBufSize() const
    {
        return canvas.totalTexels() * sizeof(short);
    }
};

