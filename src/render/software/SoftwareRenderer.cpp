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

#include "SoftwareRenderer.hpp"
#include "entity/component/TransformComponent.hpp"
#include "entity/component/CameraComponent.hpp"
#include "render/OldRenderer.hpp"
#include "engine/EngineContext.hpp"
#include "level/LevelManager.hpp"
#include "entity/Entity.hpp"
#include "render/WireframeLevelRenderer.hpp"
#include "util/StopWatch.hpp"
#include "LevelRenderer.hpp"
#include "entity/system/CameraSystem.hpp"
#include "engine/Engine.hpp"

#if 0

static void mark_lights(EngineContext* context)
{
    return;

    X_Light* lights = context->renderer->dynamicLights;
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        if(!x_light_is_free(lights + i))
        {
            x_bsplevel_mark_surfaces_light_is_close_to(context->levelManager->getCurrentLevel(), lights + i, context->frameCount);
        }
    }
}

void OldRenderer::scheduleNextLevelOfPortals(X_RenderContext& renderContext, int recursionDepth)
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

        Camera& cam = scheduledPortal->cam;

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

void OldRenderer::createCameraFromPerspectiveOfPortal(X_RenderContext& renderContext, Portal& portal, Camera& dest)
{
    calculateCameraPositionOnOtherSideOfPortal(renderContext, portal, dest);
    calculateCameraViewMatrix(renderContext, portal, dest);


    dest.viewport.viewFrustum.planes = dest.viewport.viewFrustumPlanes;

    dest.updateFrustum();
}

void OldRenderer::calculateCameraPositionOnOtherSideOfPortal(X_RenderContext& renderContext, Portal& portal, Camera& cam)
{
    cam.position = portal.transformPointToOtherSide(renderContext.cam->position);

    int leafId = renderContext.level->findLeafPointIsIn(portal.otherSide->center) - renderContext.level->leaves;
    cam.overrideBspLeaf(leafId, renderContext.level);
}

void OldRenderer::calculateCameraViewMatrix(X_RenderContext& renderContext, Portal& portal, Camera& cam)
{
    cam.viewMatrix = *renderContext.viewMatrix * portal.transformToOtherSide;

    cam.viewMatrix.dropTranslation();

    Mat4x4 translation;
    translation.loadTranslation(-cam.position);

    cam.viewMatrix = cam.viewMatrix * translation;
}

void OldRenderer::renderScheduledPortal(ScheduledPortal* scheduledPortal, EngineContext& engineContext, X_RenderContext* renderContext)
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

void OldRenderer::renderCamera(Camera* cam, EngineContext* engineContext)
{
    X_RenderContext renderContext;
    x_enginecontext_get_rendercontext_for_camera(engineContext, cam, &renderContext);

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

#endif

static void x_engine_begin_frame(EngineContext* context)
{
    ++context->frameCount;
}

static void fill_with_background_color(EngineContext* engineContext)
{
    if(engineContext->renderer->fillColor != X_RENDERER_FILL_DISABLED)
    {
        engineContext->screen->canvas.fill(engineContext->renderer->fillColor);
    }
}

static void x_renderer_begin_frame(OldRenderer* renderer, EngineContext* engineContext)
{
    renderer->totalSurfacesRendered = 0;
    renderer->currentFrame = engineContext->frameCount;
    renderer->dynamicLightsNeedingUpdated = 0xFFFFFFFF;

    renderer->totalRenderedPortals = 0;
    renderer->maxRenderedPortals = 10;
    renderer->maxPortalDepth = 1;
}

static void clear_zbuffer(EngineContext* engineContext)
{
    engineContext->screen->clearZBuf();
}

static void x_cameraobject_determine_current_bspleaf(Camera* cam, X_RenderContext* renderContext)
{
    cam->currentLeaf = renderContext->level->findLeafPointIsIn(cam->position);
}

static void x_cameraobject_load_pvs_for_current_leaf(Camera* cam, X_RenderContext* renderContext)
{
    if(cam->currentLeaf == cam->lastLeaf)
    {
        return;
    }

    cam->lastLeaf = cam->currentLeaf;

    renderContext->level->pvs.decompressPvsForLeaf(*cam->currentLeaf, cam->pvsForCurrentLeaf);
}

void x_cameraobject_render(Camera* cam, X_RenderContext* renderContext)
{
    x_assert(renderContext != NULL, "No render context");
    x_assert(renderContext->engineContext != NULL, "No engine context in render context");

    int currentFrame = x_enginecontext_get_frame(renderContext->engineContext);

    if(!cam->flags.hasFlag(CAMERA_OVERRIDE_PVS))
    {
        x_cameraobject_determine_current_bspleaf(cam, renderContext);
    }

    x_cameraobject_load_pvs_for_current_leaf(cam, renderContext);
    renderContext->level->pvs.markVisibleLeaves(cam->pvsForCurrentLeaf, currentFrame);

    renderContext->camPos = x_cameraobject_get_position(cam);
    renderContext->currentFrame = currentFrame;

    if(cam->currentLeaf != renderContext->level->leaves + 0 && !renderContext->renderer->wireframe)
    {
        LevelRenderer levelRenderer;
        levelRenderer.render(*renderContext);
    }
    else
    {
        WireframeLevelRenderer wireFrameLevelRenderer(*renderContext, 5 * 16 - 1, 15);

        wireFrameLevelRenderer.render();
    }
}

void SoftwareRenderer::render()
{
    x_engine_begin_frame(engineContext);
    x_renderer_begin_frame(engineContext->renderer, engineContext);
    clear_zbuffer(engineContext);
    fill_with_background_color(engineContext);


    if(engineContext->levelManager->getCurrentLevel() == nullptr)
    {
        return;
    }

// FIXME: 2-20-2019
#if true
    CameraSystem* cameraSystem = Engine::getInstance()->cameraSystem;   // FIXME: DI

    auto& entitiesWithCameras = cameraSystem->getAllCameras();

    for(auto& entity : entitiesWithCameras)
    {
        CameraComponent* camera = entity->getComponent<CameraComponent>();
        TransformComponent* transformComponent = entity->getComponent<TransformComponent>();

        camera->position = transformComponent->getPosition();

        transformComponent->toMat4x4(camera->viewMatrix);
        camera->updateFrustum();

        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(engineContext, camera, &renderContext);

        x_ae_context_begin_render(activeEdgeContext, &renderContext);

        StopWatch::start("traverse-level");
        x_cameraobject_render(camera, &renderContext);
        StopWatch::stop("traverse-level");

        x_ae_context_scan_edges(activeEdgeContext);
    }
#endif
}

SoftwareRenderer::SoftwareRenderer(X_AE_Context* activeEdgeContext, EngineContext* engineContext)
    : activeEdgeContext(activeEdgeContext),
      engineContext(engineContext)
{

}
