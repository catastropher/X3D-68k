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

#ifdef __nspire__
#include <libndls.h>
#endif

#include "OldRenderer.hpp"
#include "engine/EngineContext.hpp"
#include "error/Error.hpp"
#include "engine/Engine.hpp"
#include "system/File.hpp"
#include "object/CameraObject.hpp"
#include "util/StopWatch.hpp"
#include "level/Portal.hpp"
#include "entity/CameraComponent.hpp"
#include "entity/TransformComponent.hpp"
#include "entity/Entity.hpp"

static void x_renderer_init_console_vars(X_Renderer* renderer, Console* console)
{
    x_console_register_var(console, &renderer->fillColor, "render.fillColor", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->showFps, "render.showFps", X_CONSOLEVAR_BOOL, "0", 0);
    x_console_register_var(console, &renderer->frustumClip, "frustumClip", X_CONSOLEVAR_BOOL, "1", 0);
    x_console_register_var(console, &renderer->mipLevel, "mipLevel", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->renderMode, "rendermode", X_CONSOLEVAR_INT, "3", 0);
    x_console_register_var(console, &renderer->wireframe, "wireframe", X_CONSOLEVAR_BOOL, "0", 0);
    x_console_register_var(console, &renderer->maxFramesPerSecond, "maxFps", X_CONSOLEVAR_INT, "60", 0);
}

static void cmd_res(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->getConsole(), "Usage: res [WxH] -> changes screen resolution\n");
        return;
    }
    
    Screen* screen = context->getScreen();
    
    if(screen->handlers.isValidResolution == NULL)
        x_system_error("No screen valid resolution checking createEntityCallback");
    
    int w, h;
    sscanf(argv[1], "%dx%d", &w, &h);
    
    if(!screen->handlers.isValidResolution(w, h))
    {
        x_console_print(context->getConsole(), "Invalid resolution for platform\n");
        return;
    }
    
    context->getRenderer()->screenW = w;
    context->getRenderer()->screenH = h;
    
    x_console_printf(context->getConsole(), "Effect will take place on next vidrestart\n");
}

static void cmd_fov(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->getConsole(), "Usage: fov [angle] -> changes field of view\n");
        return;
    }
    
    x_fp16x16 fov = x_fp16x16_from_float(atof(argv[1]) * 256.0 / 360.0);
    
    if(fov == 0)
    {
        x_console_print(context->getConsole(), "Invalid angle\n");
        return;
    }
    
    context->getRenderer()->fov = fov;
    x_console_printf(context->getConsole(), "Effect will take place on next vidrestart\n");
}

static void cmd_vidrestart(X_EngineContext* context, int argc, char* argv[])
{
    x_enginecontext_restart_video(context);
}

static void cmd_fullscreen(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->getConsole(), "Usage: fullscreen [WxH] -> enables/disables fullscreen mode\n");
        return;
    }
    
    context->getRenderer()->fullscreen = atoi(argv[1]);
    
    x_console_printf(context->getConsole(), "Effect will take place on next vidrestart\n");;
}

#include "error/Log.hpp"

// Prints the ID of the surface we're currently looking at
static void cmd_surfid(X_EngineContext* context, int argc, char* argv[])
{
    // Render a frame so we can get the span data
    //x_engine_render_frame(context);
    
    Screen* screen = context->getScreen();
    Vec2 center = screen->getCenter();
    
    int surfaceId = x_ae_context_find_surface_point_is_in(&context->getRenderer()->activeEdgeContext, center.x, center.y, context->getCurrentLevel());
    
    if(surfaceId != -1)
        x_console_printf(context->getConsole(), "Looking at surface #%d\n", surfaceId);
    else
        x_console_print(context->getConsole(), "Not looking at a surface\n");
}

static void cmd_lighting(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->getConsole(), "Usage: lighting [1/0] -> enables/disables lighting");
        return;
    }
    
    context->getRenderer()->enableLighting = atoi(argv[1]);
    x_cache_flush(&context->getRenderer()->surfaceCache);
}

static void cmd_scalescreen(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_printf(context->getConsole(), "Usage: %s [0/1] - enables/disables screen scaling", argv[0]);
        return;
    }
    
#ifndef __nspire__
    x_console_print(context->getConsole(), "Screen scaling only available on Nspire\n");
    return;
#endif
    
//    CameraObject* cam = context->getScreen()->cameraListHead;
//    int w;
//    int h;
//
//    context->getRenderer()->scaleScreen = atoi(argv[1]) != 0;
//
//    if(context->getRenderer()->scaleScreen)
//    {
//        w = 320 / 2;
//        h = 240 / 2;
//    }
//    else
//    {
//        w = 320;
//        h = 240;
//    }
//
//    // FIXME: don't hardcode angle
//    cam->viewport.init(Vec2(0, 0), w, h, fp(X_ANG_60));
}

#define MAX_SURFACES 300
#define MAX_EDGES 1000
#define MAX_ACTIVE_EDGES 5000

static int convert_shade(int intensity, int shade)
{
    return (intensity * (shade) + 16) / 32;
}

static void init_color_shade(X_Color* colorMap, const X_Palette* palette, X_Color color)
{
    for(int i = 0; i < X_COLORMAP_SHADES_PER_COLOR; ++i)
    {
        unsigned char r, g, b;
        x_palette_get_rgb(palette, color, &r, &g, &b);
        
        int rr = X_MIN(255, convert_shade(r, i));
        int gg = X_MIN(255, convert_shade(g, i));
        int bb = X_MIN(255, convert_shade(b, i));
        
        colorMap[(int)color * X_COLORMAP_SHADES_PER_COLOR + i] = x_palette_get_closest_color_from_rgb(palette, rr, gg, bb);
    }
}

static void x_renderer_init_colormap(X_Renderer* renderer, const X_Palette* palette)
{
    renderer->colorMap = (X_Color*)x_malloc(256 * X_COLORMAP_SHADES_PER_COLOR * sizeof(X_Color));
    
    const int TOTAL_FULLBRIGHTS = 32;
    
    for(int i = 0; i < 256 - TOTAL_FULLBRIGHTS; ++i)
        init_color_shade(renderer->colorMap, palette, i);
    
    for(int i = 256 - TOTAL_FULLBRIGHTS; i < 256; ++i)
    {
        for(int j = 0; j < X_COLORMAP_SHADES_PER_COLOR; ++j)
            renderer->colorMap[i * X_COLORMAP_SHADES_PER_COLOR + j] = i;
    }    
}

static void x_renderer_init_dynamic_lights(X_Renderer* renderer)
{
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        renderer->dynamicLights[i].flags = X_LIGHT_FREE;
        renderer->dynamicLights[i].id = i;
    }
    
    renderer->dynamicLightsNeedingUpdated = 0;
}

static void x_renderer_console_cmds(Console* console)
{
    x_console_register_cmd(console, "res", cmd_res);
    x_console_register_cmd(console, "fov", cmd_fov);
    x_console_register_cmd(console, "vidrestart", cmd_vidrestart);    
    x_console_register_cmd(console, "fullscreen", cmd_fullscreen);    
    x_console_register_cmd(console, "surfid", cmd_surfid);    
    x_console_register_cmd(console, "lighting", cmd_lighting);
    x_console_register_cmd(console, "scalescreen", cmd_scalescreen);
}

static void x_renderer_set_default_values(X_Renderer* renderer, Screen* screen, int fov)
{
    renderer->screenW = screen->getW();
    renderer->screenH = screen->getH();
    renderer->videoInitialized = 0;
    renderer->fullscreen = 0;
    renderer->fov = fov;
    renderer->enableLighting = 1;
    renderer->scaleScreen = 0;
    renderer->maxFramesPerSecond = 60;
}

void x_renderer_init(X_Renderer* renderer, Console* console, Screen* screen, int fov)
{
    x_renderer_console_cmds(console);
    x_renderer_init_console_vars(renderer, console);
    x_renderer_set_default_values(renderer, screen, fov);
    
    x_cache_init(&renderer->surfaceCache, 500000 * 4, "surfacecache");     // TODO: this size should be configurable
    x_renderer_init_colormap(renderer, screen->palette);
    x_renderer_init_dynamic_lights(renderer);
}

void x_renderer_cleanup(X_Renderer* renderer)
{
    x_cache_cleanup(&renderer->surfaceCache);
    
    x_free(renderer->colorMap);
}

void x_renderer_restart_video(X_Renderer* renderer, Screen* screen)
{
    // FIXME: need to reconstruct object
    
    //x_ae_context_cleanup(&renderer->activeEdgeContext);
    //x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
}

static void x_engine_begin_frame(X_EngineContext* context)
{
    ++context->frameCount;
}

static void mark_lights(X_EngineContext* context)
{
    return;
    
    X_Light* lights = context->getRenderer()->dynamicLights;
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        if(!x_light_is_free(lights + i))
        {
            x_bsplevel_mark_surfaces_light_is_close_to(context->getCurrentLevel(), lights + i, context->frameCount);
        }
    }
}

static void fill_with_background_color(X_EngineContext* engineContext)
{
    if(engineContext->getRenderer()->fillColor != X_RENDERER_FILL_DISABLED)
        engineContext->getScreen()->canvas.fill(engineContext->getRenderer()->fillColor);
}

static void x_renderer_begin_frame(X_Renderer* renderer, X_EngineContext* engineContext)
{
    renderer->totalSurfacesRendered = 0;
    renderer->currentFrame = engineContext->frameCount;
    renderer->dynamicLightsNeedingUpdated = 0xFFFFFFFF;

    renderer->totalRenderedPortals = 0;
    renderer->maxRenderedPortals = 10;
    renderer->maxPortalDepth = 1;
}

static void clear_zbuffer(X_EngineContext* engineContext)
{
    engineContext->getScreen()->clearZBuf();
}

void X_Renderer::scheduleNextLevelOfPortals(X_RenderContext& renderContext, int recursionDepth)
{
    if(recursionDepth > maxPortalDepth)
    {
        return;
    }

    const int MAX_PORTAL_SPANS = 1024;
    auto level = renderContext.level;

    for(auto portal = level->portalHead; portal != nullptr; portal = portal->next)
    {
        if(!portal->aeSurface || portal->aeSurface->spanHead.next == nullptr)
        {
            continue;
        }

        if(scheduledPortals.isFull())
        {
            break;
        }

        auto scheduledPortal = scheduledPortals.allocate();
        //auto nextPortalSpan = Zone::alloc<PortalSpan>(MAX_PORTAL_SPANS);

        scheduledPortal->recursionDepth = recursionDepth;
        //scheduledPortal->spans = nextPortalSpan;
        scheduledPortal->cam = *renderContext.cam;
        scheduledPortal->portal = portal;

        auto otherSide = portal->otherSide;

        scheduledPortal->cam.viewMatrix = *renderContext.viewMatrix; //otherSide->orientation;
        scheduledPortal->cam.position = otherSide->center;

        CameraObject& cam = scheduledPortal->cam;

        createCameraFromPerspectiveOfPortal(renderContext, *portal, cam);

        // for(auto span = portal->aeSurface->spanHead.next; span != nullptr; span = span->next)
        // {
        //     nextPortalSpan->left = span->x1;
        //     nextPortalSpan->right = span->x2;
        //     nextPortalSpan->y = span->y;

        //     ++nextPortalSpan;
        // }

        // scheduledPortal->spansEnd = nextPortalSpan;
    }
}

void X_Renderer::createCameraFromPerspectiveOfPortal(X_RenderContext& renderContext, Portal& portal, CameraObject& dest)
{
    calculateCameraPositionOnOtherSideOfPortal(renderContext, portal, dest);
    calculateCameraViewMatrix(renderContext, portal, dest);
    

    dest.viewport.viewFrustum.planes = dest.viewport.viewFrustumPlanes;

    dest.updateFrustum();
}

void X_Renderer::calculateCameraPositionOnOtherSideOfPortal(X_RenderContext& renderContext, Portal& portal, CameraObject& cam)
{
    cam.position = portal.transformPointToOtherSide(renderContext.cam->position);

    int leafId = renderContext.level->findLeafPointIsIn(portal.otherSide->center) - renderContext.level->leaves;
    cam.overrideBspLeaf(leafId, renderContext.level);
}

void X_Renderer::calculateCameraViewMatrix(X_RenderContext& renderContext, Portal& portal, CameraObject& cam)
{
    cam.viewMatrix = *renderContext.viewMatrix * portal.transformToOtherSide;

    cam.viewMatrix.dropTranslation();

    Mat4x4 translation;
    translation.loadTranslation(-cam.position);

    cam.viewMatrix = cam.viewMatrix * translation;
}

void X_Renderer::renderScheduledPortal(ScheduledPortal* scheduledPortal, X_EngineContext& engineContext, X_RenderContext* renderContext)
{

    bool wireframe = renderContext->renderer->wireframe;

    renderContext->renderer->wireframe = false;

    x_engine_begin_frame(&engineContext);
    x_enginecontext_get_rendercontext_for_camera(&engineContext, &scheduledPortal->cam, renderContext);

    x_ae_context_begin_render(&activeEdgeContext, renderContext);

    x_cameraobject_render(&scheduledPortal->cam, renderContext);

    x_ae_context_scan_edges(&activeEdgeContext);

    //Zone::free(scheduledPortal->spans);

    scheduledPortal->spans = nullptr;

    renderContext->renderer->wireframe = wireframe;
}

void X_Renderer::renderCamera(CameraObject* cam, X_EngineContext* engineContext)
{
    X_RenderContext renderContext;
    x_enginecontext_get_rendercontext_for_camera(engineContext, cam, &renderContext);
    
    //if((renderer->renderMode & 2) != 0)
    x_ae_context_begin_render(&activeEdgeContext, &renderContext);
    
    StopWatch::start("traverse-level");
    x_cameraobject_render(cam, &renderContext);
    StopWatch::stop("traverse-level");

    x_ae_context_scan_edges(&activeEdgeContext);

    int recursionDepth = 1;

    x_enginecontext_get_rendercontext_for_camera(engineContext, cam, &renderContext);

    do
    {
        scheduleNextLevelOfPortals(renderContext, recursionDepth);

        if(scheduledPortals.isEmpty() || ++totalRenderedPortals > maxRenderedPortals)
        {
            break;
        }

        auto scheduledPortal = scheduledPortals.dequeue();
        renderScheduledPortal(scheduledPortal, *engineContext, &renderContext);

        recursionDepth = scheduledPortal->recursionDepth + 1;
    } while(true);

    while(!scheduledPortals.isEmpty())
    {
        auto scheduledPortal = scheduledPortals.dequeue();
        //Zone::free(scheduledPortal->spans);

        scheduledPortal->spans = nullptr;
    }
}

void x_renderer_render_frame(X_EngineContext* engineContext)
{
    x_engine_begin_frame(engineContext);
    x_renderer_begin_frame(engineContext->getRenderer(), engineContext);
    clear_zbuffer(engineContext);
    fill_with_background_color(engineContext);
    mark_lights(engineContext);


    if(engineContext->getCurrentLevel() == nullptr)
    {
        return;
    }

    Array<CameraComponent> cameras = CameraComponent::getAll();
    
    for(auto& camera : cameras)
    {
        TransformComponent* transformComponent = camera.owner->getComponent<TransformComponent>();

        camera.position = transformComponent->getPosition();

        // FIXME: remove angles from camera
        camera.angleX = 0;
        camera.angleY = 0;
        camera.updateView();
        transformComponent->toMat4x4(camera.viewMatrix);

        engineContext->getRenderer()->renderCamera(&camera, engineContext);
    }
}

