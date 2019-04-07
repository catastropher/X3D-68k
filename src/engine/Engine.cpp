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
#include "entity/component/InputComponent.hpp"
#include "render/StatusBar.hpp"
#include "entity/component/PhysicsComponent.hpp"
#include "level/LevelManager.hpp"
#include "render/software/SoftwareRenderer.hpp"
#include "hud/MessageQueue.hpp"
#include "hud/OverlayRenderer.hpp"
#include "hud/EntityOverlay.hpp"

EngineContext Engine::instance;
bool Engine::wasInitialized = false;
bool Engine::isDone = false;

void x_enginecontext_get_rendercontext_for_camera(EngineContext* engineContext, Camera* cam, X_RenderContext* dest)
{
    dest->cam = cam;
    dest->camPos = x_cameraobject_get_position(cam);
    dest->canvas = &engineContext->screen->canvas;
    dest->zbuf = engineContext->screen->zbuf;
    dest->currentFrame = x_enginecontext_get_frame(engineContext);
    dest->engineContext = engineContext;
    dest->level = engineContext->levelManager->getCurrentLevel();
    dest->renderer = engineContext->renderer;
    dest->screen = engineContext->screen;
    dest->viewFrustum = &cam->viewport.viewFrustum;
    dest->viewMatrix = &cam->viewMatrix;
    dest->renderer = engineContext->renderer;
}

static void initSystem(SystemConfig& config)
{
    FileSystem::init(config.programPath);
    Log::init(config.logFile, config.enableLogging);
    MemoryManager::init(config.hunkSize, config.zoneSize);
}

void initEngineContext(EngineContext* context, X_Config& config)
{
    context->queue = new EngineQueue(context);
    context->mainFont = new Font;
    context->keyState = new KeyState;

    context->frameCount = 1;    // TODO: belongs in the renderer
    context->lastFrameStart = context->frameStart;

    context->screen = new Screen(
        config.screen->w,
        config.screen->h,
        config.screen->screenHandlers,
        config.screen->palette);

    if(!context->mainFont->loadFromFile(config.font))
    {
        x_system_error("Failed to load font");
    }

    context->console = new Console(context, context->mainFont);
    context->renderer = new OldRenderer(
        context->screen,
        context->console,
        config.screen->fov.toFp16x16());

    context->entityManager = new EntityManager;

    context->brushModelSystem = new BrushModelSystem;
    context->entityManager->registerEntitySystem(context->brushModelSystem);

    context->cameraSystem = new CameraSystem;
    context->entityManager->registerEntitySystem(context->cameraSystem);

    context->boxColliderSystem = new BoxColliderSystem;
    context->entityManager->registerEntitySystem(context->boxColliderSystem);

    context->inputSystem = new InputSystem;
    context->entityManager->registerEntitySystem(context->inputSystem);

    context->renderSystem = new RenderSystem;
    context->entityManager->registerEntitySystem(context->renderSystem);

    context->scriptableSystem = new ScriptableSystem;
    context->entityManager->registerEntitySystem(context->scriptableSystem);

    context->levelManager = new LevelManager(*context->queue, *context->entityManager);

    context->mouseState = new MouseState(context->console, context->screen);

    context->messageQueue = new MessageQueue(Duration::fromSeconds(5.0_fp), context->screen, context->mainFont);

    context->overlayRenderer = new OverlayRenderer(*context->console);
    context->entityOverlay = new EntityOverlay("entity", context->screen);

    context->overlayRenderer->addOverlay(context->entityOverlay);
}

EngineContext* Engine::init(X_Config& config)
{
    if(wasInitialized)
    {
        x_system_error("Called Engine::init() after engine already initialized");
    }

    initSystem(config.systemConfig);

    x_memory_init();
    x_filesystem_init(config.path);
    x_filesystem_add_search_path("../assets");

    x_platform_init(&instance, &config);
    initEngineContext(&instance, config);

    auto platform = instance.getPlatform();
    platform->init(config);
    
    wasInitialized = true;
    
    return &instance;
}

void Engine::shutdownEngine()
{
    x_platform_cleanup(&instance);
    x_filesystem_cleanup();
    x_memory_free_all();
    x_log_cleanup();
}

void Engine::quit()
{
    isDone = true;
}

static void lockToFrameRate(EngineContext* engineContext)
{
    Duration frameDuration = Clock::getTicks() - engineContext->frameStart;

    if(frameDuration == Duration::fromSeconds(0))
    {
        engineContext->estimatedFramesPerSecond = fp::fromInt(1000);

        return;
    }

    engineContext->estimatedFramesPerSecond = fp::fromInt(1) / frameDuration.toSeconds();

    fp maxFramesPerSecond = fp::fromInt(engineContext->renderer->maxFramesPerSecond);

    if(engineContext->estimatedFramesPerSecond > maxFramesPerSecond)
    {
        Duration targetFrameLength =  Duration::fromSeconds(fp::fromInt(1) / engineContext->renderer->maxFramesPerSecond);

        Duration sleepMilliseconds = targetFrameLength - frameDuration;

        Clock::delay(sleepMilliseconds);

        frameDuration = Clock::getTicks() - engineContext->frameStart;
        engineContext->estimatedFramesPerSecond = fp::fromInt(1) / frameDuration.toSeconds();
    }
}

void handle_console_keys(EngineContext* context)
{
    x_keystate_handle_key_repeat(context->keyState, context->frameStart);

    KeyCode key;
    while(x_keystate_dequeue(context->keyState, &key))
    {
        if(key == KeyCode::backtick)
        {
            x_console_close(context->console);
            x_keystate_reset_keys(context->keyState);
            x_keystate_disable_text_input(context->keyState);

            return;
        }

        x_console_send_key(context->console, key);
    }
}

bool handle_console(EngineContext* engineContext)
{
    if(engineContext->console->isOpen())
    {
        handle_console_keys(engineContext);

        return true;
    }

    if(x_keystate_key_down(engineContext->keyState, KeyCode::backtick))
    {
        x_console_open(engineContext->console);
        x_keystate_reset_keys(engineContext->keyState);
        x_keystate_enable_text_input(engineContext->keyState);

        return false;
    }

    return false;
}

void sendInputUpdate(const InputUpdate& update)
{
    InputSystem* inputSystem = Engine::getInstance()->inputSystem;  // FIXME: DI

    auto& inputComponents = inputSystem->getAllEntities();

    for(auto& entity : inputComponents)
    {
        InputComponent* inputComponent = entity->getComponent<InputComponent>();

        if(inputComponent->handler != nullptr)
        {
            if(inputComponent->handler(entity, update))
            {
                break;
            }
        }
    }
}

static void runFrame(EngineContext* engineContext)
{
    // FIXME: why is this function responsible for this?
    x_platform_handle_keys(engineContext);
    x_platform_handle_mouse(engineContext);

    // FIXME: temp until we figure out where this should be done
    if(x_keystate_key_down(engineContext->keyState, KeyCode::escape))
    {
        Engine::quit();
    }

    engineContext->lastFrameStart = engineContext->frameStart;
    engineContext->frameStart = Clock::getTicks();
    engineContext->timeDelta = (engineContext->frameStart - engineContext->lastFrameStart).toSeconds();

    PhysicsEngine::update(*engineContext->levelManager->getCurrentLevel(), engineContext->timeDelta);

    // Move the brush models to where their transform says they are
    auto& brushModels = engineContext->brushModelSystem->getAllEntities();

    for(auto& entity : brushModels)
    {
        BrushModelPhysicsComponent* brushModelPhysicsComponent = entity->getComponent<BrushModelPhysicsComponent>();

        brushModelPhysicsComponent->model->center = entity->getComponent<TransformComponent>()->getPosition();
    }

    // Send time updates to scritable components
    auto& scriptableEntities = engineContext->scriptableSystem->getAllEntities();

    Time currentTime = Clock::getTicks();
    EntityUpdate entityUpdate(currentTime, engineContext->timeDelta, engineContext);


    for(Entity* entity : scriptableEntities)
    {
        ScriptableComponent* scriptableComponent = entity->getComponent<ScriptableComponent>();

        if(scriptableComponent->update != nullptr && currentTime >= scriptableComponent->nextUpdateTime)
        {
            scriptableComponent->update(*entity, entityUpdate);
        }
    }


    lockToFrameRate(engineContext);

    Console* console = engineContext->console;

    //x_renderer_render_frame(engineContext);
    SoftwareRenderer softwareRenderer(&engineContext->renderer->activeEdgeContext, engineContext);
    softwareRenderer.render();

    engineContext->overlayRenderer->render();

    // Commented out for implementing message queue.
    //StatusBar::render(engineContext->screen->canvas, *engineContext->mainFont);
    engineContext->messageQueue->render();

    if(console->isOpen())
    {
        x_console_render(console);
    }

    if(!handle_console(engineContext))
    {
        InputUpdate update(engineContext->keyState, engineContext->frameStart, engineContext->timeDelta);
        sendInputUpdate(update);
    }

    engineContext->getPlatform()->getScreenDriver().update(engineContext->screen);
}

void Engine::run()
{
    x_console_execute_cmd(instance.console, "exec user.cfg");

    while(!isDone)
    {
        runFrame(&instance);
    }

    shutdownEngine();
}

