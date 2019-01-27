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

#include <unistd.h>

#include "Engine.hpp"
#include "error/Error.hpp"
#include "system/File.hpp"
#include "error/Log.hpp"
#include "render/RenderContext.hpp"
#include "geo/Polygon3.hpp"
#include "platform/Platform.hpp"
#include "memory/Memory.hpp"
#include "util/X_JsonParser.hpp"
#include "system/FileSystem.hpp"
#include "physics/PhysicsEngine.hpp"
#include "system/Clock.hpp"
#include "entity/EntityManager.hpp"

X_EngineContext Engine::instance;
bool Engine::wasInitialized = false;
bool Engine::isDone = false;

static void initSystem(SystemConfig& config)
{
    FileSystem::init(config.programPath);
    Log::init(config.logFile, config.enableLogging);
    MemoryManager::init(config.hunkSize, config.zoneSize);
}

X_EngineContext* Engine::init(X_Config& config)
{
    if(wasInitialized)
    {
        x_system_error("Called Engine::init() after engine already initialized");
    }

    initSystem(config.systemConfig);

    instance.init();

    x_memory_init();
    x_filesystem_init(config.path);
    x_filesystem_add_search_path("../assets");

    x_platform_init(&instance, &config);
    x_enginecontext_init(&instance, &config);

    auto platform = instance.getPlatform();
    platform->init(config);
    
    wasInitialized = true;
    
    return &instance;
}

void Engine::shutdownEngine()
{
    x_platform_cleanup(&instance);
    x_enginecontext_cleanup(&instance);
    x_filesystem_cleanup();
    x_memory_free_all();
    x_log_cleanup();
}

void Engine::quit()
{
    isDone = true;
}

static void lockToFrameRate(X_EngineContext* engineContext)
{
    int diff = Clock::getTicks() - engineContext->frameStart;

    if(diff == 0)
    {
        engineContext->estimatedFramesPerSecond = fp::fromInt(1000);

        return;
    }

    engineContext->estimatedFramesPerSecond = fp::fromInt(1000) / diff;

    fp maxFramesPerSecond = fp::fromInt(engineContext->getRenderer()->maxFramesPerSecond);

    if(engineContext->estimatedFramesPerSecond > maxFramesPerSecond)
    {
        fp millisecondsPerFrame = fp::fromInt(1000) / engineContext->estimatedFramesPerSecond;
        fp targetMillisecondsPerFrame = fp::fromInt(1000) / engineContext->getRenderer()->maxFramesPerSecond;

        fp sleepMilliseconds = targetMillisecondsPerFrame - millisecondsPerFrame;

        Clock::delay(sleepMilliseconds.toInt());

        diff = Clock::getTicks() - engineContext->frameStart;
        engineContext->estimatedFramesPerSecond = fp::fromInt(1000) / diff;
    }
}

static void runFrame(X_EngineContext* engineContext)
{
    x_platform_handle_keys(engineContext);

    if(x_keystate_key_down(engineContext->getKeyState(), X_KEY_ESCAPE))
    {
        Engine::quit();
    }

    engineContext->lastFrameStart = engineContext->frameStart;
    engineContext->frameStart = Clock::getTicks();
    engineContext->timeDelta = fp::fromInt(engineContext->frameStart - engineContext->lastFrameStart) / 1000;

    PhysicsEngine::update(*engineContext->getCurrentLevel(), engineContext->timeDelta);
    engineContext->entityManager->updateEntities(Clock::getTicks());

    lockToFrameRate(engineContext);

    x_renderer_render_frame(engineContext);

    engineContext->getPlatform()->getScreenDriver().update(engineContext->getScreen());
}

void Engine::run()
{
    while(!isDone)
    {
        runFrame(&instance);
    }

    shutdownEngine();
}

