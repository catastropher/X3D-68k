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

#include "Screen.hpp"
#include "Camera.hpp"
#include "dev/console/Console.hpp"
#include "engine/Engine.hpp"

void Screen::restartVideo(int newW, int newH, fp newFov)
{
    canvas.resize(newW, newH);
    
    x_free(zbuf);
    zbuf = (x_fp0x16*)x_malloc(calculateZBufSize());
    
    // FIXME: Broadcast to cameras the change so they can update their viewports
//    for(Camera* cam = cameraListHead; cam != NULL; cam = cam->nextInCameraList)
//    {
//        if(cam->screenResizeCallback != nullptr)
//        {
//            cam->screenResizeCallback(cam, this, newFov);
//        }
//    }
}

Screen::Screen(int w, int h, ScreenEventHandlers& handlers, const X_Palette* palette_)
    : palette(palette_)
{
    canvas.resize(w, h);
    zbuf = (x_fp0x16*)x_malloc(calculateZBufSize());

    this->handlers = handlers;
}
