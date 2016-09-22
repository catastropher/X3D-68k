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

#include "X3D_common.h"
#include "X3D_plane.h"
#include "X3D_polygon.h"
#include "X3D_vector.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"
#include "render/geo/X3D_render_polygon.h"

#define FIXDIV8(_n, _d) (((long)(_n) << 8) / (_d))

#define FIXMULN(_a, _b, _n) (((long)(_a) * (_b)) >> (_n))

#define FIXMUL8(_a, _b) FIXMULN(_a, _b, 8)
#define FIXMUL15(_a, _b) FIXMULN(_a, _b, 15)

// Calculates the normals of the unrotated planes of the view frustum
void calculate_frustum_plane_normals(X3D_Frustum* f) {
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    
    int16 w = screenman->w + 20;
    int16 h = screenman->h + 20;
    int16 dist = screenman->scale_x;
    
    X3D_Vex3D top_left = { -w / 2, -h / 2, dist };
    X3D_Vex3D top_right = { w / 2, -h / 2, dist };
    
    X3D_Vex3D bottom_left = { -w / 2, h / 2, dist };
    X3D_Vex3D bottom_right = { w / 2, h / 2, dist };
    
    X3D_Vex3D cam_pos = { 0, 0, 0 };
    
    // Top plane
    x3d_plane_construct_from_three_points(&f->p[1], &cam_pos, &top_right, &top_left);
    
    // Bottom plane
    x3d_plane_construct_from_three_points(&f->p[2], &cam_pos, &bottom_left, &bottom_right);
    
    // Left plane
    x3d_plane_construct_from_three_points(&f->p[3], &cam_pos, &top_left, &bottom_left);
    
    // Right plane
    x3d_plane_construct_from_three_points(&f->p[4], &cam_pos, &bottom_right, &top_right);
    
    // Near plane
    x3d_plane_construct_from_three_points(&f->p[0], &bottom_right, &top_right, &top_left);
    
    f->p[0].normal = (X3D_Vex3D){ 0, 0, 32767 };
    
    // Hack...
    f->p[0].d = 15;//c->dist - DIST_TO_NEAR_PLANE;
    
    f->total_p = 5;
}

// Calculates the distance from each plane in the view frustum to the origin
void calculate_frustum_plane_distances(X3D_Frustum* f, X3D_CameraObject* cam) {
    const int PLAYER_HEIGHT = 0;
    X3D_Vex3D cam_pos;
    
    x3d_object_pos(cam, &cam_pos);
    
    X3D_Vex3D eye_cam_pos = { cam_pos.x, cam_pos.y - PLAYER_HEIGHT, cam_pos.z };
    
    uint16 i;
    for(i = 0; i < f->total_p; ++i) {
        f->p[i].d = x3d_vex3d_fp0x16_dot(&f->p[i].normal, &eye_cam_pos);
    }
    
    X3D_Vex3D out;
    x3d_dynamicobject_forward_vector(cam, &out);
    
    short dist = 15;//c->dist - DIST_TO_NEAR_PLANE;
    
    out.x = ((int32)out.x * dist) >> X3D_NORMAL_BITS;
    out.y = ((int32)out.y * dist) >> X3D_NORMAL_BITS;
    out.z = ((int32)out.z * dist) >> X3D_NORMAL_BITS;
    
    out.x += cam_pos.x;
    out.y += cam_pos.y - PLAYER_HEIGHT;
    out.z += cam_pos.z;
    
    f->p[0].d = x3d_vex3d_fp0x16_dot(&f->p[0].normal, &out);  
}

// Calculates the rotated plane normals of the view frustum
void calculate_frustum_rotated_normals(X3D_Frustum* src, X3D_Frustum* f, X3D_CameraObject* cam) {
    int16 i, d;
    
    X3D_Mat3x3 transpose = cam->base.mat;
    x3d_mat3x3_transpose(&transpose);
    
    for(i = 0; i < src->total_p; ++i) {
        X3D_Vex3D n = src->p[i].normal;
        
        n.x >>= 1;
        n.y >>= 1;
        n.z >>= 1;
        
        x3d_vex3d_int16_rotate(&f->p[i].normal, &n, &transpose);
        
        f->p[i].normal.x <<= 1;
        f->p[i].normal.y <<= 1;
        f->p[i].normal.z <<= 1;
    }
    
    // Near plane normal
    x3d_dynamicobject_forward_vector(cam, &f->p[0].normal);

    f->total_p = src->total_p;
}

void x3d_rasterpolygon3d_clip_add_point(X3D_RasterPolygon3D* poly, X3D_PolygonRasterVertex3D* v) {
  poly->v[poly->total_v++] = *v;
}

///////////////////////////////////////////////////////////////////////////////
/// Clips a 3D polygon to a plane.
///
/// @param poly
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasterpolygon3d_clip_to_plane(X3D_RasterPolygon3D* poly, X3D_Plane* plane, X3D_RasterPolygon3D* dest) {
  int16 next_v;
  int16 v = poly->total_v - 1;

  dest->total_v = 0;
  
  int16 dist;
  int16 next_dist = x3d_plane_point_distance(plane, &poly->v[v].v);
  
  for(next_v = 0; next_v < poly->total_v; v = next_v, ++next_v) {
    dist      = next_dist;
    next_dist = x3d_plane_point_distance(plane, &poly->v[next_v].v);
    
    _Bool in      = dist > 0;
    _Bool next_in = next_dist > 0;

    if(in)
      x3d_rasterpolygon3d_clip_add_point(dest, poly->v + v);
    
    if(in != next_in) {
      int16 in  = abs(dist);
      int16 out = abs(next_dist);
      
      float t = (float)in / (in + out);//((int32)in << 15) / (in + out);
      
      X3D_PolygonRasterVertex3D new_v;
      
      new_v.v = x3d_vex3d_make(
        x3d_linear_interpolate_float(poly->v[v].v.x, poly->v[next_v].v.x, t),
        x3d_linear_interpolate_float(poly->v[v].v.y, poly->v[next_v].v.y, t),
        x3d_linear_interpolate_float(poly->v[v].v.z, poly->v[next_v].v.z, t)
        );
      
      new_v.uu = x3d_linear_interpolate_float(poly->v[v].uu, poly->v[next_v].uu, t);
      new_v.vv = x3d_linear_interpolate_float(poly->v[v].vv, poly->v[next_v].vv, t);
      
      new_v.lu = x3d_linear_interpolate_float(poly->v[v].lu, poly->v[next_v].lu, t);
      new_v.lv = x3d_linear_interpolate_float(poly->v[v].lv, poly->v[next_v].lv, t);
      
      new_v.intensity = x3d_linear_interpolate_float(poly->v[v].intensity, poly->v[next_v].intensity, t);
      
      x3d_rasterpolygon3d_clip_add_point(dest, &new_v);
    }
  }
  
  return poly->total_v > 2;
}

_Bool x3d_rasterpolygon3d_clip_to_frustum(X3D_RasterPolygon3D* src, X3D_Frustum* f, X3D_RasterPolygon3D* dest) {
    X3D_RasterPolygon3D temp[2];
    temp[0].v = alloca(1000);
    temp[1].v = alloca(1000);
    
    int16 current_temp = 0;
    X3D_RasterPolygon3D* poly = src;
    
    if(f->total_p == 0)
        return 0;
        
    int16 i;
    for(i = 0; i < f->total_p - 1; i++) {
        if(!x3d_rasterpolygon3d_clip_to_plane(poly, &f->p[i], &temp[current_temp])) {
            dest->total_v = 0;
            return 0;
        }
        
        poly = &temp[current_temp];
        current_temp ^= 1;
    }

    return x3d_rasterpolygon3d_clip_to_plane(poly, &f->p[f->total_p - 1], dest);
}

X3D_Frustum* x3d_get_view_frustum(X3D_CameraObject* cam) {
    static X3D_Frustum frustum;
    static X3D_Plane planes[5];
    
    frustum.p = planes;
    
    X3D_Frustum f = {
        .p = alloca(1000)
    };
    
    calculate_frustum_plane_normals(&f);
    calculate_frustum_rotated_normals(&f, &frustum, cam);
    calculate_frustum_plane_distances(&frustum, cam);
    
    return &frustum;
}

void test_clip(X3D_Polygon3D* poly, X3D_CameraObject* cam) {
    return;
    
    X3D_Frustum f = {
        .p = alloca(1000)
    };

    X3D_Frustum f2 = {
        .p = alloca(1000)
    };

    X3D_RasterPolygon3D poly3d = {
        .v = alloca(1000)
    };
    
    X3D_RasterPolygon3D poly3d_out = {
        .v = alloca(1000)
    };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        poly3d.v[i].v = poly->v[i];
    }
    
    poly3d.total_v = poly->total_v;
  
    //x3d_log(X3D_INFO, "Vex: %d %d %d", f2.p[3].normal.x, f2.p[3].normal.y, f2.p[3].normal.z);
  
    clip_polygon_to_frustum(&poly3d, &f2, &poly3d_out);
  
    X3D_PolygonRasterVertex2D v[poly3d_out.total_v];
    
    for(i = 0; i < poly3d_out.total_v; ++i) {
        X3D_Vex3D temp;
        x3d_camera_transform_points(cam, &poly3d_out.v[i].v, 1, &temp, NULL);
        x3d_vex3d_int16_project(&v[i].v, &temp);
    }
    
    static int total = 0;
  
    X3D_PolygonRasterAtt at = {
        .flat = {
            .color = (total % 2 ? x3d_rgb_to_color(0, 0, 255) : 31)
        }
    };
    
    ++total;
  
  x3d_polygon2d_render_flat(v, poly3d_out.total_v, &at);
  
  
  //x3d_polygon3d_copy(&c, poly);
}

