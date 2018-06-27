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

#include "engine/X_Engine.h"
#include "engine/X_init.h"
#include "util/X_StopWatch.hpp"

template<typename T>
class Game
{
public:
    Game(X_Config& config_) : done(false), config(config_)
    {
        engineContext = x_engine_init(&config);
    }

    void run()
    {
        static_cast<T*>(this)->init();
        static_cast<T*>(this)->gameloop();
        static_cast<T*>(this)->cleanup();
    }

protected:
    X_EngineContext* getInstance() const
    {
        return engineContext;
    }

    void render()
    {
        static_cast<T*>(this)->renderView();
        static_cast<T*>(this)->renderHud();
    }

    void renderView()
    {

    }

    void renderHud()
    {

    }

    void updateScreen()
    {
        auto engineContext = getInstance();
        auto screenDriver = engineContext->getPlatform()->getScreenDriver();
        screenDriver.update(engineContext->getScreen());
    }

    void handleKeys()
    {

    }

    void init()
    {

    }

    void cleanup()
    {

    }

    void gameloop()
    {
        do
        {
            StopWatch::start("total");

            static_cast<T*>(this)->handleKeys();

            StopWatch::start("rendering");

            static_cast<T*>(this)->renderView();

            StopWatch::stop("rendering");

            static_cast<T*>(this)->renderHud();

            StopWatch::start("update-screen");

            static_cast<T*>(this)->updateScreen();

            StopWatch::stop("update-screen");

            StopWatch::stop("total");
        } while(!done);
    }

    X_Config& getConfig()
    {
        return config;
    }

    bool done;

private:
    X_EngineContext* engineContext;
    X_Config& config;
};

