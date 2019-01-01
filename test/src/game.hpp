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

#include <X3D/X3D.h>
#include <cmath>

#include "Context.h"
#include "init.h"
#include "render.h"
#include "keys.h"
#include "Player.hpp"

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
        setupPlayer();
        
        context.player = &player;
        context.engineContext = getInstance();
        
        ::init(&context, nullptr, getConfig());

        x_console_register_cmd(context.engineContext->getConsole(), "stopwatch", StopWatch::stopwatchCmd);

        //x_console_register_var(context.engineContext->getConsole(), &player.getCollider().position, "cam.pos", X_CONSOLEVAR_VEC3, "0 0 0", false);

        x_console_execute_cmd(context.engineContext->getConsole(), "cam.pos -289,-162,192");
        

        x_gameobjectloader_load_objects(getInstance(), getInstance()->getCurrentLevel()->entityDictionary);
    }

    void renderView()
    {
        auto& camera = player.getCamera();
        
        camera.angleX = player.angleX.toFp16x16();
        camera.angleY = player.angleY.toFp16x16();
        
        camera.position = player.getTransform().getPosition();
        
        camera.updateView();

        Vec3fp pos = player.getTransform().getPosition();
        Vec3fp vel = player.getCollider().velocity;

        StatusBar::setItem(
            "position",
            "%f %f %f\n",
            pos.x.toFloat(),
            pos.y.toFloat(),
            pos.z.toFloat());

        StatusBar::setItem(
            "velocity",
            "%f %f %f\n",
            vel.x.toFloat(),
            vel.y.toFloat(),
            vel.z.toFloat());
        
        ::render(&context);
    }

    void handleKeys()
    {
        ::handle_keys(&context);

        if(x_keystate_key_down(getInstance()->getKeyState(), X_KEY_ESCAPE))
        {
            done = true;
        }

        static bool shot = false;

        if(x_keystate_key_down(getInstance()->getKeyState(), (X_Key)'f') || !shot)
        {
            if(bluePortal == nullptr)
            {
                bluePortal = getInstance()->getCurrentLevel()->addPortal();
                bluePortal->poly.vertices = bluePortalVertices;

                auto palette = getInstance()->getScreen()->palette;
                //bluePortal->enableOutline(palette->darkBlue);
            }

            shootPortal(bluePortal);
            Portal::linkMutual(orangePortal, bluePortal);
        }

        shot = true;

        if(x_keystate_key_down(getInstance()->getKeyState(), (X_Key)'g'))
        {
            if(orangePortal == nullptr)
            {
                orangePortal = getInstance()->getCurrentLevel()->addPortal();
                orangePortal->poly.vertices = orangePortalVertices;

                auto palette = getInstance()->getScreen()->palette;
                X_Color orange = x_palette_get_closest_color_from_rgb(palette, 255, 69, 0);

                //orangePortal->enableOutline(orange);
            }

            shootPortal(orangePortal);
            Portal::linkMutual(orangePortal, bluePortal);
        }
    }

    void shootPortal(Portal* portal);

    void setupPlayer()
    {
        X_EngineContext* engineContext = getInstance();
        
        player.camera.viewport.init((X_Vec2) { 0, 0 }, x_screen_w(engineContext->getScreen()), x_screen_h(engineContext->getScreen()), fp(X_ANG_60));
        x_screen_attach_camera(engineContext->getScreen(), &player.getCamera());
        
        player.angleX = 0;
        player.angleY = 0;
        player.getTransform().setPosition(Vec3fp(0, fp::fromInt(-50), fp::fromInt(-800)));
        player.getCollider().velocity = Vec3fp(0, 0, 0);
    }

    Player player;
    Context context;

    friend class Game<TestGame>;
};

