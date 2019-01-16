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
#include "object/X_CameraObject.h"

void X_Screen::attachCamera(X_CameraObject* camera)
{
    camera->nextInCameraList = NULL;
    
    if(!cameraListHead)
    {
        cameraListHead = camera;
        return;
    }
    
    X_CameraObject* currentCam = cameraListHead;
    while(currentCam->nextInCameraList)
    {
        currentCam = currentCam->nextInCameraList;
    }
    
    currentCam->nextInCameraList = camera;
}

void X_Screen::detachCamera(X_CameraObject* camera)
{
    X_CameraObject* currentCam = cameraListHead;
    while(currentCam && currentCam->nextInCameraList != camera)
    {
        currentCam = currentCam->nextInCameraList;
    }
    
    bool camWasInList = currentCam != nullptr;
    if(!camWasInList)
    {
        return;
    }
    
    currentCam->nextInCameraList = currentCam->nextInCameraList->nextInCameraList;
}

void X_Screen::restartVideo(int newW, int newH, x_fp16x16 newFov)
{
    canvas.resize(newW, newH);
    
    x_free(zbuf);
    zbuf = (x_fp0x16*)x_malloc(calculateZBufSize());
    
    // Broadcast to cameras the change so they can update their viewports
    for(X_CameraObject* cam = cameraListHead; cam != NULL; cam = cam->nextInCameraList)
    {
        if(cam->screenResizeCallback != nullptr)
        {
            cam->screenResizeCallback(cam, this, newFov);
        }
    }
}
