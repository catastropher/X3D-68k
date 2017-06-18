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

#include "X_Viewport.h"
#include "math/X_trig.h"
#include "util/X_util.h"

static inline int calculate_distance_to_projection_plane(int w, x_angle256 fieldOfView)
{
    return x_fp16x16_make(w / 2) / x_tan(fieldOfView / 2);
}

void x_viewport_init(X_Viewport* viewport, X_Vec2 screenPos, int w, int h, x_angle256 fieldOfView)
{
    viewport->screenPos = screenPos;
    viewport->w = w;
    viewport->h = h;
    viewport->distToNearPlane = calculate_distance_to_projection_plane(w, fieldOfView);
    
    /// @todo If we add a far plane, this should be 6
    viewport->viewFrustum.totalPlanes = 5;
    viewport->viewFrustum.planes = viewport->viewFrustumPlanes;
}

static inline X_Vec3 calculate_center_of_near_plane(const X_Viewport* viewport, const X_Vec3* camPos, const X_Vec3_fp16x16* forward)
{
    X_Vec3_fp16x16 translationAs16x16 = x_vec3_scale(forward, viewport->distToNearPlane);
    X_Vec3 translation = x_vec3_fp16x16_to_vec3(&translationAs16x16);
    
    return x_vec3_add(camPos, &translation);
}

static inline X_Vec3 calculate_right_translation(const X_Viewport* viewport, const X_Vec3_fp16x16* right)
{
    X_Vec3_fp16x16 translationAs16x16 = x_vec3_scale(right, viewport->w / 2);
    return x_vec3_fp16x16_to_vec3(&translationAs16x16);
}

static inline X_Vec3 calculate_up_translation(const X_Viewport* viewport, const X_Vec3_fp16x16* up)
{
    X_Vec3_fp16x16 translationAs16x16 = x_vec3_scale(up, viewport->h / 2);
    return x_vec3_fp16x16_to_vec3(&translationAs16x16);
}

void x_viewport_update_frustum(X_Viewport* viewport, const X_Vec3* camPos, const X_Vec3_fp16x16* forward, const X_Vec3_fp16x16* right, const X_Vec3_fp16x16* up)
{
    X_Vec3 nearPlaneCenter = calculate_center_of_near_plane(viewport, camPos, forward);
    
    X_Vec3 rightTranslation = calculate_right_translation(viewport, right);
    X_Vec3 leftTranslation = x_vec3_neg(&rightTranslation);
    
    X_Vec3 upTranslation = calculate_up_translation(viewport, up);
    X_Vec3 downTranslation = x_vec3_neg(&upTranslation);
    
    X_Vec3 nearPlaneVertices[4] =
    {
        x_vec3_add_three(&nearPlaneCenter, &rightTranslation, &upTranslation),      // Top right
        x_vec3_add_three(&nearPlaneCenter, &rightTranslation, &downTranslation),    // Bottom right
        x_vec3_add_three(&nearPlaneCenter, &leftTranslation, &downTranslation),     // Bottom left
        x_vec3_add_three(&nearPlaneCenter, &leftTranslation, &upTranslation)        // Top left
    };
    
    // Top, bottom, left, and right planes
    for(int i = 0; i < 4; ++i)
    {
        int next = (i != 3 ? i + 1 : 0);
        x_plane_init_from_three_points(viewport->viewFrustumPlanes + i, nearPlaneVertices + i, camPos, nearPlaneVertices + next);
    }
    
    // Near plane
    int distToNearPlane = 16;
    X_Vec3_fp16x16 translationAs16x16 = x_vec3_scale(forward, distToNearPlane);
    X_Vec3 translation = x_vec3_fp16x16_to_vec3(&translationAs16x16);
    X_Vec3 pointOnNearPlane = x_vec3_add(&translation, camPos);
    
    x_plane_init_from_normal_and_point(viewport->viewFrustumPlanes + 4, forward, &pointOnNearPlane);
    
    x_vec3_print(forward, "Forward");
    x_frustum_print(&viewport->viewFrustum);
    
}

void x_viewport_project(const X_Viewport* viewport, const X_Vec3* src, X_Vec2* dest)
{
    dest->x = src->x * viewport->distToNearPlane / src->z + viewport->w / 2;
    dest->y = src->y * viewport->distToNearPlane / src->z + viewport->h / 2;
}

void x_viewport_clamp_vec2(const X_Viewport* viewport, X_Vec2* v)
{
    v->x = X_MAX(v->x, viewport->screenPos.x);
    v->x = X_MIN(v->x, viewport->screenPos.x + viewport->w - 1);
    
    v->y = X_MAX(v->y, viewport->screenPos.y);
    v->y = X_MIN(v->y, viewport->screenPos.y + viewport->h - 1);
}

