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

static inline int calculate_distance_to_projection_plane(int w, x_fp16x16 fieldOfView)
{
    return x_fp16x16_from_int(w / 2) / x_tan(fp(fieldOfView) / 2).toFp16x16();
}

static void x_viewport_init_mip_distances(X_Viewport* viewport)
{
    x_fp16x16 mipScale[3] =
    {
        x_fp16x16_from_float(1.0),
        x_fp16x16_from_float(0.5 * 0.8),
        x_fp16x16_from_float(0.25 * 0.8)
    };

    // TODO: this will need to take into account y scale once it's added
    x_fp16x16 xScale = x_fp16x16_from_int(viewport->distToNearPlane);
    for(int i = 0; i < 3; ++i)
    {
        viewport->mipDistances[i] = x_fp16x16_div(xScale, mipScale[i]);
        printf("Mip Distance: %f\n", x_fp16x16_to_float(viewport->mipDistances[i]));
    }

    // screen->w / z = 1.0
}

void x_viewport_init(X_Viewport* viewport, X_Vec2 screenPos, int w, int h, x_fp16x16 fieldOfView)
{
    viewport->screenPos = screenPos;
    viewport->w = w;
    viewport->h = h;
    viewport->distToNearPlane = calculate_distance_to_projection_plane(w, fieldOfView);

    /// @todo If we add a far plane, this should be 6
    viewport->viewFrustum.totalPlanes = 5;
    viewport->viewFrustum.planes = viewport->viewFrustumPlanes;

    x_viewport_init_mip_distances(viewport);
}

static inline Vec3 calculate_center_of_near_plane(const X_Viewport* viewport, const Vec3* camPos, const Vec3* forward)
{
    Vec3 translation = x_vec3_scale_int(forward, viewport->distToNearPlane);
    return *camPos + translation;
}

static inline Vec3 calculate_right_translation(const X_Viewport* viewport, const Vec3* right)
{
    return x_vec3_scale_int(right, viewport->w / 2 + 10);
}

static inline Vec3 calculate_up_translation(const X_Viewport* viewport, const Vec3* up)
{
    return x_vec3_scale_int(up, viewport->h / 2 + 10);
}

void x_viewport_update_frustum(X_Viewport* viewport, const Vec3* camPos, const Vec3* forward, const Vec3* right, const Vec3* up)
{
    Vec3 nearPlaneCenter = calculate_center_of_near_plane(viewport, camPos, forward);

    Vec3 rightTranslation = calculate_right_translation(viewport, right);
    Vec3 leftTranslation = x_vec3_neg(&rightTranslation);

    Vec3 upTranslation = calculate_up_translation(viewport, up);
    Vec3 downTranslation = x_vec3_neg(&upTranslation);

    Vec3 nearPlaneVertices[4] =
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
    Vec3 translation = x_vec3_scale_int(forward, distToNearPlane);
    Vec3 pointOnNearPlane = translation + *camPos;

    x_plane_init_from_normal_and_point(viewport->viewFrustumPlanes + 4, forward, &pointOnNearPlane);
}

void x_viewport_project_vec3(const X_Viewport* viewport, const Vec3* src, X_Vec2_fp16x16* dest)
{
    // TODO: may be able to get away with multiplying by distToNearPlane / z
    dest->x = x_fp16x16_div(src->x, src->z) * viewport->distToNearPlane + x_fp16x16_from_int(viewport->w) / 2;
    dest->y = x_fp16x16_div(src->y, src->z) * viewport->distToNearPlane + x_fp16x16_from_int(viewport->h) / 2;;
}

void x_viewport_clamp_vec2(const X_Viewport* viewport, X_Vec2* v)
{
    v->x = X_MAX(v->x, viewport->screenPos.x);
    v->x = X_MIN(v->x, viewport->screenPos.x + viewport->w - 1);

    v->y = X_MAX(v->y, viewport->screenPos.y);
    v->y = X_MIN(v->y, viewport->screenPos.y + viewport->h - 1);
}

void x_viewport_clamp_vec2_fp16x16(const X_Viewport* viewport, X_Vec2_fp16x16* v)
{
    v->x = X_MAX(v->x, x_fp16x16_from_int(viewport->screenPos.x));
    v->x = X_MIN(v->x, x_fp16x16_from_int(viewport->screenPos.x + viewport->w) - X_FP16x16_ONE);

    v->y = X_MAX(v->y, x_fp16x16_from_int(viewport->screenPos.y));
    v->y = X_MIN(v->y, x_fp16x16_from_int(viewport->screenPos.y + viewport->h) - X_FP16x16_ONE);
}
