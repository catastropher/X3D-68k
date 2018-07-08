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

#include <X3D/X3D.h>

#include "Context.h"
#include "init.h"
#include "render.h"
#include "keys.h"

class TestGame : public Game<TestGame>
{
public:
    TestGame(X_Config& config) : Game<TestGame>(config)
    {

    }

    Portal* orangePortal = nullptr;
    Portal* bluePortal = nullptr;

    Vec3fp bluePortalVertices[16];
    Vec3fp orangePortalVertices[16];

private:
    void init()
    {
        createCamera();

        context.cam = cam;
        context.engineContext = getInstance();
        
        ::init(&context, nullptr, getConfig());

        x_console_register_cmd(context.engineContext->getConsole(), "stopwatch", StopWatch::stopwatchCmd);

        x_console_register_var(context.engineContext->getConsole(), &cam->collider.position, "cam.pos", X_CONSOLEVAR_VEC3, "0 0 0", false);

        x_console_execute_cmd(context.engineContext->getConsole(), "cam.pos -289,-162,192");
    }

    void renderView()
    {
        ::render(&context);
    }

    void handleKeys()
    {
        ::handle_keys(&context);

        if(x_keystate_key_down(getInstance()->getKeyState(), X_KEY_ESCAPE))
        {
            done = true;
        }

        if(x_keystate_key_down(getInstance()->getKeyState(), (X_Key)'f'))
        {
            if(bluePortal == nullptr)
            {
                bluePortal = getInstance()->getCurrentLevel()->addPortal();
                bluePortal->poly.vertices = bluePortalVertices;
            }

            shootPortal(bluePortal);
            Portal::linkMutual(orangePortal, bluePortal);
        }

        if(x_keystate_key_down(getInstance()->getKeyState(), (X_Key)'g'))
        {
            if(orangePortal == nullptr)
            {
                orangePortal = getInstance()->getCurrentLevel()->addPortal();
                orangePortal->poly.vertices = orangePortalVertices;
            }

            shootPortal(orangePortal);
            Portal::linkMutual(orangePortal, bluePortal);
        }
    }

    void shootPortal(Portal* portal);

    void createCamera()
    {
        X_EngineContext* engineContext = getInstance();

        cam = x_cameraobject_new(engineContext);
        cam->viewport.init((X_Vec2) { 0, 0 }, x_screen_w(engineContext->getScreen()), x_screen_h(engineContext->getScreen()), fp(X_ANG_60));
        x_screen_attach_camera(engineContext->getScreen(), cam);
        //context->cam->screenResizeCallback = cam_screen_size_changed_callback;
        
        cam->angleX = 0;
        cam->angleY = 0;
        cam->collider.position = Vec3(0, -50 * 65536, -800 * 65536);
        cam->collider.velocity = x_vec3_origin();
        
        x_cameraobject_update_view(cam);
    }

    X_CameraObject* cam;
    Context context;

    friend class Game<TestGame>;
};

