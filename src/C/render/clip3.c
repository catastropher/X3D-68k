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

#define FIXDIV8(_n, _d) (((long)(_n) << 8) / (_d))

#define FIXMULN(_a, _b, _n) (((long)(_a) * (_b)) >> (_n))

#define FIXMUL8(_a, _b) FIXMULN(_a, _b, 8)
#define FIXMUL15(_a, _b) FIXMULN(_a, _b, 15)

// Clips a polygon against a plane. Returns whether a valid polygon remains.
_Bool clip_polygon_to_plane(X3D_Polygon3D* poly, X3D_Plane* plane, X3D_Polygon3D* dest) {
        short i;
        short next_point;
        short in, next_in;
        short dot, next_dot;
        short t;
        
        short out[10];
        short out_pos = 0;
        
        dot = x3d_vex3d_fp0x16_dot(&poly->v[0], &plane->normal);
        
        in = (dot >= plane->d);
        
        short total_outside = !in;
        
        dest->total_v = 0;
        
        x3d_assert(poly->total_v > 1);
        
        short clipped = 0;
        
        for(i = 0; i < poly->total_v; i++) {
                if(clipped == 2 && 0) {
                        // A convex polygon can at most have two edges clipped, so if we've reached it
                        // just copy over the other ones (assuming we're back to being inside the poly)
                        
                        if(in) {
                                for(; i < poly->total_v; i++) {
                                        dest->v[dest->total_v++] = poly->v[i];
                                }
                        }
                        
                        break;
                        
                }
                
                
                next_point = i + 1;
                if(next_point == poly->total_v)
                        next_point = 0;
                
                // The vertex is inside the plane, so don't clip it
                if(in) {
                        dest->v[dest->total_v++] = poly->v[i];
                }
                        
                //errorif(!in, "Point not in!");
                        
                        
                next_dot = x3d_vex3d_fp0x16_dot(&poly->v[next_point], &plane->normal);
                next_in = (next_dot >= plane->d);
                
                total_outside += !next_in;
                
                // The points are on opposite sides of the plane, so clip it
                if(in != next_in) {
                        ++clipped;
                        
                        // Scale factor to get the point on the plane
                        
                        t = FIXDIV8(plane->d - dot, next_dot - dot);
                        //t = fast_div_fix(plane->d - dot, next_dot - dot, 15 - 8);
                        
                        
                        //errorif(t == 0, "t == 0");
                        
                        if(t == 0) {
                                //printf("T == 0\n");
                        }
                        
                        //printf("Dist: %d\n", dot + plane->d);
                        //printf("T: %ld Z: %d\n", t, poly->v[i].z);
                        
                        dest->v[dest->total_v].x = poly->v[i].x + FIXMUL8(((long)poly->v[next_point].x - poly->v[i].x), t);
                        dest->v[dest->total_v].y = poly->v[i].y + FIXMUL8(((long)poly->v[next_point].y - poly->v[i].y), t);
                        dest->v[dest->total_v].z = poly->v[i].z + FIXMUL8(((long)poly->v[next_point].z - poly->v[i].z), t);
                        dest->total_v++;
                }
                
                if(next_in != in) {
                        out[out_pos++] = dest->total_v - 1;
                }
                
                dot = next_dot;
                in = next_in;
        }
        
        return dest->total_v > 1;       
}

// Clips a polygon against the entire view frustum
// This routine requires two temporary polygons, one of which the
// final polygon will be in. This returns the address of which one it
// is
char clip_polygon_to_frustum(X3D_Polygon3D* src, X3D_Frustum* f, X3D_Polygon3D* dest) {
#if 1
        X3D_Polygon3D temp[2];
        
        temp[0].v = alloca(1000);
        temp[1].v = alloca(1000);
        
        int current_temp = 0;
        X3D_Polygon3D* poly = src;
        int i;
        
        //xassert(f->total_p != 0);
        
        //xassert(f->total_p < MAX_PLANES);
        
        if(f->total_p == 0)
                return 0;
        
        
#if 1
        for(i = 0; i < f->total_p - 1; i++) {
                //errorif(poly->total_v < 3, "Invalid clip poly");
                //return clip_polygon_to_plane(src, &f->p[FRUSTUM_TOP], dest);
                
                //if(i == PLANE_LEFT || i == PLANE_RIGHT)
                //      continue;
                        
                if(!clip_polygon_to_plane(poly, &f->p[i], &temp[current_temp])) {
                        dest->total_v = 0;
                        return 0;
                }
                
                poly = &temp[current_temp];
                current_temp = !current_temp;
        }
#endif
        
        //return poly->total_v > 2;
        return clip_polygon_to_plane(poly, &f->p[f->total_p - 1], dest);
        
#endif
}

// Calculates the normals of the unrotated planes of the view frustum
void calculate_frustum_plane_normals(X3D_Frustum* f) {
  X3D_ScreenManager* screenman = x3d_screenmanager_get();
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  int16 w = screenman->w + 20;
  int16 h = screenman->h + 20;
  int16 dist = screenman->scale_x;
  
  X3D_Vex3D top_left = { -w / 2, -h / 2, dist };
  X3D_Vex3D top_right = { w / 2, -h / 2, dist };
  
  X3D_Vex3D bottom_left = { -w / 2, h / 2, dist };
  X3D_Vex3D bottom_right = { w / 2, h / 2, dist };
  
  X3D_Vex3D cam_pos = { 0, 0, 0 };
  
  //error("ERROR\n");
  
#if 1
  // Top plane
  x3d_plane_construct(&f->p[1], &cam_pos, &top_right, &top_left);
  
  // Bottom plane
  x3d_plane_construct(&f->p[2], &cam_pos, &bottom_left, &bottom_right);
  
  // Left plane
  x3d_plane_construct(&f->p[3], &cam_pos, &top_left, &bottom_left);
  
  // Right plane
  x3d_plane_construct(&f->p[4], &cam_pos, &bottom_right, &top_right);
  
  // Near plane
  x3d_plane_construct(&f->p[0], &bottom_right, &top_right, &top_left);
  
  f->p[0].normal = (X3D_Vex3D){ 0, 0, 32767 };
  
  // Hack...
  f->p[0].d = 15;//c->dist - DIST_TO_NEAR_PLANE;
#else
  test_construct_frustum_from_polygon3D(c, &c->frustum_unrotated);
  c->frustum_unrotated.p[4].normal = (Vex3D){0, 0, 32767};
  c->frustum_unrotated.p[0].d = 15;
  //exit(-1);
#endif
  
  f->total_p = 5;
}

// Calculates the distance from each plane in the view frustum to the origin
void calculate_frustum_plane_distances(X3D_Frustum* f, X3D_CameraObject* cam) {
  int i;
  
  const int PLAYER_HEIGHT = 0;
  X3D_Vex3D cam_pos;
  
  x3d_object_pos(cam, &cam_pos);
  
  X3D_Vex3D eye_cam_pos = { cam_pos.x, cam_pos.y - PLAYER_HEIGHT, cam_pos.z };
  
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

  //printf("P: %d\nNear plane normal: ", c->frustum_unrotated.total_p);
  //print_vex3d(&c->frustum.p[4].normal);
  
  f->total_p = src->total_p;
}

void test_clip(X3D_Polygon3D* poly, X3D_CameraObject* cam) {
  X3D_Frustum f = {
    .p = alloca(1000)
  };

  X3D_Frustum f2 = {
    .p = alloca(1000)
  };
  
  calculate_frustum_plane_normals(&f);
  calculate_frustum_rotated_normals(&f, &f2, cam);
  calculate_frustum_plane_distances(&f2, cam);
  
  X3D_Polygon3D c = {
    .v = alloca(1000)
  };
  
  x3d_log(X3D_INFO, "Vex: %d %d %d", f2.p[3].normal.x, f2.p[3].normal.y, f2.p[3].normal.z);
  
  clip_polygon_to_frustum(poly, &f2, &c);
  x3d_polygon3d_copy(&c, poly);
}



