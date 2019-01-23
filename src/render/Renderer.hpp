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

#include "Texture.hpp"
#include "dev/console/Console.hpp"
#include "ActiveEdge.hpp"
#include "Light.hpp"
#include "memory/CircularQueue.hpp"
#include "object/CameraObject.hpp"

#define X_RENDERER_FILL_DISABLED -1

#define X_RENDERER_MAX_LIGHTS 32

struct PortalSpan
{
    short left;
    short right;
    int y;
};

struct ScheduledPortal
{
    Portal* portal;
    PortalSpan* spans;
    PortalSpan* spansEnd;

    X_CameraObject cam;
    int recursionDepth;
};

typedef struct X_Renderer
{
    X_Renderer(X_Screen* screen) : activeEdgeContext(2000, 1000, 20000, screen) { }

    void scheduleNextLevelOfPortals(X_RenderContext& renderContext, int recursionDepth);
    void renderScheduledPortal(ScheduledPortal* scheduledPortal, X_EngineContext& engineContext, X_RenderContext* renderContext);
    void renderCamera(X_CameraObject* cam, X_EngineContext* engineContext);
    
    X_AE_Context activeEdgeContext;
    X_Cache surfaceCache;
    
    X_Light dynamicLights[X_RENDERER_MAX_LIGHTS];
    unsigned int dynamicLightsNeedingUpdated;
    
    X_Color* colorMap;
    
    int currentFrame;
    
    int fillColor;
    bool showFps;
    
    int screenW;
    int screenH;
    x_fp16x16 fov;
    bool fullscreen;
    bool videoInitialized;
    bool frustumClip;              // debug
    bool enableLighting;
    
    bool scaleScreen;
    
    int renderMode;
    
    int totalSurfacesRendered;

    int mipLevel;
    x_fp16x16 mipDistances[X_BSPTEXTURE_MIP_LEVELS - 1];
    
    bool wireframe;

    int totalRenderedPortals;
    int maxRenderedPortals;
    int maxPortalDepth;
    
    int maxFramesPerSecond;

    CircularQueue<ScheduledPortal, 10> scheduledPortals;

private:
    static void createCameraFromPerspectiveOfPortal(X_RenderContext& renderContext, Portal& portal, X_CameraObject& dest);
    static void calculateCameraViewMatrix(X_RenderContext& renderContext, Portal& portal, X_CameraObject& cam);

    static void calculateCameraPositionOnOtherSideOfPortal(X_RenderContext& renderContext, Portal& portal, X_CameraObject& cam);

} X_Renderer;

#define X_COLORMAP_SHADES_PER_COLOR 64

static inline X_Color x_renderer_get_shaded_color(X_Renderer* renderer, X_Color color, int intensity)
{
    return renderer->colorMap[(int)color * X_COLORMAP_SHADES_PER_COLOR + intensity];
}

void x_renderer_init(X_Renderer* renderer, Console* console, X_Screen* screen, int fov);
void x_renderer_cleanup(X_Renderer* renderer);

void x_renderer_restart_video(X_Renderer* renderer, X_Screen* screen);

void x_renderer_render_frame(struct X_EngineContext* engineContext);

