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

#include "X_Screen.h"

void x_screen_attach_camera(X_Screen* screen, X_CameraObject* camera)
{
    camera->nextInCameraList = NULL;
    
    if(!screen->cameraListHead)
    {
        screen->cameraListHead = camera;
        return;
    }
    
    X_CameraObject* currentCam = screen->cameraListHead;
    while(currentCam->nextInCameraList)
        currentCam = currentCam->nextInCameraList;
    
    currentCam->nextInCameraList = camera;
}

void x_screen_detach_camera(X_Screen* screen, X_CameraObject* camera)
{
    X_CameraObject* currentCam = screen->cameraListHead;
    while(currentCam && currentCam->nextInCameraList != camera)
        currentCam = currentCam->nextInCameraList;
    
    _Bool camWasInList = currentCam != NULL;
    if(!camWasInList)
        return;
    
    currentCam->nextInCameraList = currentCam->nextInCameraList->nextInCameraList;
}

void x_screen_restart_video(X_Screen* screen, int newW, int newH, x_fp16x16 newFov)
{
    x_canvas_cleanup(&screen->canvas);
    x_canvas_init(&screen->canvas, newW, newH);
    
    // Broadcast to cameras the change so they can update their viewports
    for(X_CameraObject* cam = screen->cameraListHead; cam != NULL; cam = cam->nextInCameraList)
    {
        if(cam->screenResizeCallback != NULL)
            cam->screenResizeCallback(cam, screen, newFov);
    }
}
