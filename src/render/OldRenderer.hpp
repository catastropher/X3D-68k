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
#include "Camera.hpp"

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

    Camera cam;
    int recursionDepth;
};

struct OldRenderer
{
    OldRenderer(Screen* screen, Console* console, int fov);

    void scheduleNextLevelOfPortals(X_RenderContext& renderContext, int recursionDepth);
    void renderScheduledPortal(ScheduledPortal* scheduledPortal, EngineContext& engineContext, X_RenderContext* renderContext);
    void renderCamera(Camera* cam, EngineContext* engineContext);
    
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
    static void createCameraFromPerspectiveOfPortal(X_RenderContext& renderContext, Portal& portal, Camera& dest);
    static void calculateCameraViewMatrix(X_RenderContext& renderContext, Portal& portal, Camera& cam);

    static void calculateCameraPositionOnOtherSideOfPortal(X_RenderContext& renderContext, Portal& portal, Camera& cam);

};

#define X_COLORMAP_SHADES_PER_COLOR 64

static inline X_Color x_renderer_get_shaded_color(OldRenderer* renderer, X_Color color, int intensity)
{
    return renderer->colorMap[(int)color * X_COLORMAP_SHADES_PER_COLOR + intensity];
}

void x_renderer_cleanup(OldRenderer* renderer);

void x_renderer_restart_video(OldRenderer* renderer, Screen* screen);

void x_renderer_render_frame(struct EngineContext* engineContext);

