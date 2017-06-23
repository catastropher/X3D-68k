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

#include "X_CubeObject.h"
#include "engine/X_EngineContext.h"
#include "util/X_util.h"
#include "geo/X_Polygon3.h"
#include "error/X_error.h"

static void calculate_face_normals(X_CubeObject* cube)
{
    X_Vec3 faceVertices[4];
    X_Polygon3 face = x_polygon3_make(faceVertices, 4);
    X_Plane plane;
    
    for(int i = 0; i < 6; ++i)
    {
        x_cube_get_face(&cube->geometry, i, &face);
        x_plane_init_from_three_points(&plane, faceVertices + 0, faceVertices + 1, faceVertices + 2);
        
        cube->normals[i] = plane.normal;
    }    
}

static void update_orientation(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    X_Quaternion spinVelocity = x_quaternion_make(cube->angularVelocity.x, cube->angularVelocity.y, cube->angularVelocity.z, 0);
    X_Quaternion spinTemp = x_quaternion_scale(&cube->orientation, X_FP16x16_HALF);
    X_Quaternion spin;
    
    x_quaternion_mul(&spinTemp, &spinVelocity, &spin);
    spin = x_quaternion_scale(&spin, deltaTime);
    
    cube->orientation = x_quaternion_add(&cube->orientation, &spin);
    x_quaternion_normalize(&cube->orientation);
}

static void update_geometry(X_CubeObject* cube)
{
    x_cube_init(&cube->geometry, cube->size.x, cube->size.y, cube->size.z);
    
    X_Mat4x4 mat;
    x_quaternion_to_mat4x4(&cube->orientation, &mat);
    
    mat.elem[0][3] = cube->center.x;
    mat.elem[1][3] = cube->center.y;
    mat.elem[2][3] = cube->center.z;
    
    x_cube_transform(&cube->geometry, &cube->geometry, &mat);
    calculate_face_normals(cube);
}

static void reset_forces(X_CubeObject* cube)
{
    cube->torque = x_vec3_make(0, 0, 0);
    cube->force = x_vec3_make(0, 0, 0);
}

static void calculate_inverse_static_inertia(X_CubeObject* cube)
{
    int x = cube->size.x;
    int y = cube->size.y;
    int z = cube->size.z;
    
    int xx = (x * x);
    int yy = (y * y);
    int zz = (z * z);
    int massOverThree = cube->mass / 3;
    
    int inertiaX = (yy + zz) * massOverThree;
    int inertiaY = (xx + zz) * massOverThree;
    int inertiaZ = (xx + yy) * massOverThree;
    
    X_Mat4x4 inertia =
    {
        {
            { inertiaX, 0, 0, 0 },
            { 0, inertiaY, 0, 0 },
            { 0, 0, inertiaZ, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    x_mat4x4_invert_diagonal(&inertia, &cube->inverseStaticInertia);
    x_mat4x4_print_machine_readable(&cube->inverseStaticInertia);
}

static void calculate_inverse_inertia(X_CubeObject* cube)
{
    X_Mat4x4 orientation;
    x_quaternion_to_mat4x4(&cube->orientation, &orientation);
    
    X_Mat4x4 temp;
    x_mat4x4_mul(&cube->inverseStaticInertia, &orientation, &temp);
    
    x_mat4x4_transpose_3x3(&orientation);
    x_mat4x4_mul(&orientation, &temp, &cube->inverseInertia);
}

X_CubeObject* x_cubeobject_new(X_EngineContext* context, X_Vec3 pos, int width, int height, int depth, int mass)
{
    X_CubeObject* cube = (X_CubeObject*)x_gameobject_new(context, sizeof(X_CubeObject));
    
    cube->center = x_vec3_make(pos.x << 16, pos.y << 16, pos.z << 16);
    cube->size = x_vec3_make(width, height, depth);
    
    cube->angularVelocity = x_vec3_make(0, 0, 0);
    cube->linearVelocity = x_vec3_make(0, 0, 0);
    cube->orientation = x_quaternion_identity();
    
    cube->mass = mass;
    cube->invMass = x_fp16x16_div(X_FP16x16_ONE, mass);
    
    calculate_inverse_static_inertia(cube);
    reset_forces(cube);
    update_geometry(cube);
    
    return cube;
}

static void update_position(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->center = x_vec3_add_scaled(&cube->center, &cube->linearVelocity, deltaTime);
}

static void update_velocity(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->linearVelocity = x_vec3_add_scaled(&cube->linearVelocity, &cube->force, x_fp16x16_mul(deltaTime, cube->invMass));
    
    if(x_vec3_fp16x16_length(&cube->linearVelocity) < (int)(.01 * 65536))
        cube->linearVelocity = x_vec3_origin();
    
    X_Vec3_fp16x16 dAngularVelocity;
    x_mat4x4_transform_vec3_fp16x16(&cube->inverseInertia, &cube->torque, &dAngularVelocity);    
    cube->angularVelocity = x_vec3_add_scaled(&cube->angularVelocity, &dAngularVelocity, deltaTime);
    
    if(x_vec3_fp16x16_length(&cube->angularVelocity) < (int)(.01 * 65536))
        cube->angularVelocity = x_vec3_origin();
}

static int determine_point_furthest_along_normal(X_CubeObject* cube, X_Vec3_fp16x16* normal)
{
    int maxDot = -1;
    int maxVertex = -1;
    
    for(int i = 0; i < 8; ++i)
    {
        int dot = x_vec3_dot(normal, cube->geometry.vertices + i);
        
        if(dot > maxDot)
        {
            maxDot = dot;
            maxVertex = i;
        }
    }
    
    return maxVertex;
}

static int determine_incident_face(X_CubeObject* cube, X_Vec3_fp16x16* normal)
{
    int furthestPoint = determine_point_furthest_along_normal(cube, normal);
    int touchingFaces[3];
    
    x_cube_get_faces_containing_vertex(&cube->geometry, furthestPoint, touchingFaces);
    
    x_fp16x16 maxDot = -X_FP16x16_ONE;
    int maxFace = -1;
    
    for(int i = 0; i < 3; ++i)
    {
        x_fp16x16 dot = x_vec3_fp16x16_dot(normal, cube->normals + touchingFaces[i]);

        if(dot > maxDot)
        {
            maxDot = dot;
            maxFace = touchingFaces[i];
        }
    }
    
    x_assert(maxDot >= 0, "No face with positive dot");
    
    return maxFace;
}

static void calculate_velocity_of_point(const X_CubeObject* cube, const X_Vec3* point, X_Vec3_fp16x16* velocityDest)
{
    X_Vec3 center = x_vec3_fp16x16_to_vec3(&cube->center);
    X_Vec3 relativeToCenter = x_vec3_sub(point, &center);
    X_Vec3 rotationalVelocity = x_vec3_cross(&cube->angularVelocity, &relativeToCenter);
    
    *velocityDest = x_vec3_add(&cube->linearVelocity, &rotationalVelocity);
}

typedef struct Collision
{
    X_Vec3 r0;
    X_Vec3 r1;
    X_Vec3 normal;
    X_Vec3 dv;
    x_fp16x16 elasticity;
} Collision;

static void init_collision(Collision* c, X_CubeObject* cube, X_Vec3 pointOfContact, X_Vec3_fp16x16* normal, x_fp16x16 elasticity)
{
    X_Vec3 center = x_vec3_fp16x16_to_vec3(&cube->center);
    c->r0 = x_vec3_sub(&pointOfContact, &center);
    c->r1 = x_vec3_origin();
    c->normal = *normal;
    c->elasticity = elasticity;
    
    X_Vec3_fp16x16 v0;
    calculate_velocity_of_point(cube, &pointOfContact, &v0);
    
    c->dv = v0;
}

static x_fp16x16 calculate_impulse(X_CubeObject* cube, x_fp16x16 deltaTime, Collision* c)
{
    x_fp16x16 inverseMass0 = cube->invMass;
    x_fp16x16 inverseMass1 = 0;     // Infinite mass
    
    X_Vec3_fp16x16 temp = x_vec3_cross(&c->r0, &c->normal);
    X_Vec3_fp16x16 temp2;
    x_mat4x4_transform_vec3_fp16x16(&cube->inverseInertia, &temp, &temp2);
    
    X_Vec3_fp16x16 temp3 = x_vec3_cross(&temp2, &c->r0);
    
    x_fp16x16 constraintMass = inverseMass0 + inverseMass1 + x_vec3_fp16x16_dot(&c->normal, &temp3);
    x_fp16x16 n = -x_fp16x16_mul(X_FP16x16_ONE + c->elasticity, x_vec3_fp16x16_dot(&c->dv, &c->normal));
    
    return x_fp16x16_div(n, constraintMass);
}

extern X_RenderContext* g_renderContext;
extern _Bool g_Pause;

static _Bool cube_is_colliding_with_xz_plane(X_CubeObject* cube)
{
    _Bool collide = 0;
    
    for(int i = 0; i < 8; ++i)
    {
        if(cube->geometry.vertices[i].y > 0)
            collide = 1;
    }
    
    return collide;
}

static void calculate_clipped_incident_face(X_CubeObject* cube, X_Vec3_fp16x16* normal, X_Polygon3* clipped)
{
    int incidentFaceId = determine_incident_face(cube, normal);
    
    X_Vec3 incidentV[4];
    X_Polygon3 incident = x_polygon3_make(incidentV, 4);
    x_cube_get_face(&cube->geometry, incidentFaceId, &incident);
    
    X_Vec3 pointOnPlane = x_vec3_make(0, 0, 0);
    X_Plane ground;
    x_plane_init_from_normal_and_point(&ground, normal, &pointOnPlane);
    
    //x_polygon3_clip_to_plane(&incident, &ground, &clipped);
    
    clipped->totalVertices = 0;
    for(int i = 0; i < incident.totalVertices; ++i)
    {
        if(incident.vertices[i].y > 0)
            clipped->vertices[clipped->totalVertices++] = incident.vertices[i];
    }
}

static int calculate_max_penetration_depth(X_CubeObject* cube)
{
    int maxPen = 0;
    
    for(int i = 0; i < 8; ++i)
    {
        if(cube->geometry.vertices[i].y > 0)
        {
            maxPen = X_MAX(maxPen, cube->geometry.vertices[i].y);
        }
    }
    
    return maxPen;
}

static void apply_impulse_to_point(X_CubeObject* cube, x_fp16x16 deltaTime, Collision* c, x_fp16x16 impulse)
{
    x_fp16x16 normalCoefficient = x_fp16x16_mul(impulse, cube->invMass);        

    X_Vec3_fp16x16 dAdd = x_vec3_origin();
    dAdd = x_vec3_add_scaled(&dAdd, &c->normal, normalCoefficient);
    
    X_Vec3_fp16x16 scaledNormal = x_vec3_make(0, x_fp16x16_mul(c->normal.y, impulse), 0);
    X_Vec3 temp = x_vec3_cross(&c->r0, &scaledNormal);
    
    X_Vec3 dAngular;
    x_mat4x4_transform_vec3_fp16x16(&cube->inverseInertia, &temp, &dAngular);
    
    cube->linearVelocity = x_vec3_add(&cube->linearVelocity, &dAdd);
    cube->angularVelocity = x_vec3_add(&cube->angularVelocity, &dAngular);
}

static _Bool calculate_friction_direction(X_CubeObject* cube, Collision* c, X_Vec3* dir)
{
    *dir = x_vec3_add_scaled(&c->dv, &c->normal, -x_vec3_fp16x16_dot(&c->dv, &c->normal));
    
    if(x_vec3_fp16x16_length(dir) < (int)(.001 * 65536))
        return 0;
    
    x_vec3_fp16x16_normalize(dir);
    //*dir = x_vec3_neg(dir);
    x_vec3_fp16x16_print(dir, "Friction dir");
    
    return 1;
}

static _Bool handle_collision_with_xz_plane(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    X_Vec3_fp16x16 normal = x_vec3_make(0, X_FP16x16_ONE, 0);
    
    if(!cube_is_colliding_with_xz_plane(cube))
        return 0;
    
    X_Vec3 clippedV[8];
    X_Polygon3 clipped = x_polygon3_make(clippedV, 8);
    calculate_clipped_incident_face(cube, &normal, &clipped);
    
    if(clipped.totalVertices == 0)
        return 0;
    
    x_polygon3_render_wireframe(&clipped, g_renderContext, 254);
    
    //int maxPen = calculate_max_penetration_depth(cube);
    
    X_Vec3_fp16x16 resolutionNormal = x_vec3_neg(&normal);
    
    for(int i = 0; i < clipped.totalVertices; ++i)
    {
        Collision c;
        init_collision(&c, cube, clipped.vertices[i], &resolutionNormal, .1 * 65536);
        x_fp16x16 collisionImpulse = calculate_impulse(cube, deltaTime, &c);
        
        if(collisionImpulse < 0)
            collisionImpulse = 0;
        
        apply_impulse_to_point(cube, deltaTime, &c, collisionImpulse);
        
        X_Vec3_fp16x16 frictionDir;
        
        if(!calculate_friction_direction(cube, &c, &frictionDir))
            continue;
            
        c.normal = frictionDir;
        c.elasticity = 0;
        
        x_fp16x16 frictionImpulse = calculate_impulse(cube, deltaTime, &c);
        x_fp16x16 frictionCoefficient = .01 * 65536;
        x_fp16x16 maxFriction = x_fp16x16_mul(frictionCoefficient, frictionImpulse);
        
        frictionImpulse = x_fp16x16_mul(frictionImpulse, frictionCoefficient); //x_fp16x16_clamp(x_fp16x16_mul(frictionImpulse, frictionCoefficient), -maxFriction, maxFriction);
        
        printf("Friction impulse: %f\n", x_fp16x16_to_float(frictionImpulse));
        
        apply_impulse_to_point(cube, deltaTime, &c, frictionImpulse);
    }
    
    return 1;
}
    
void x_cubeobject_update(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->linearVelocity.y += 65536 * 1;
    int count = 0;
    _Bool collide;
    
    do {
        calculate_inverse_inertia(cube);
        update_velocity(cube, deltaTime);
        reset_forces(cube);
        update_position(cube, deltaTime);
        update_orientation(cube, deltaTime);
        update_geometry(cube);
        
        for(int i = 0; i < 5; ++i)
            collide = handle_collision_with_xz_plane(cube, deltaTime);
        
        ++count;
    } while(collide && count < 1);
    
    float speed = (float)cube->linearVelocity.x * cube->linearVelocity.x + (float)cube->linearVelocity.y * cube->linearVelocity.y + (float)cube->linearVelocity.z * cube->linearVelocity.z;
}

void x_cubeobject_render(X_CubeObject* cube, X_RenderContext* rcontext, X_Color color)
{
    x_cube_render(&cube->geometry, rcontext, color);
}

void x_cubeobject_apply_force(X_CubeObject* cube, X_Vec3_fp16x16 force, X_Vec3 pointOfContact)
{
    cube->force = x_vec3_add(&cube->force, &force);
    
    X_Vec3 center = x_vec3_fp16x16_to_vec3(&cube->center);
    X_Vec3 pointRelative = x_vec3_sub(&pointOfContact, &center);
    X_Vec3_fp16x16 torque = x_vec3_cross(&pointRelative, &force);
    
    cube->torque = x_vec3_add(&cube->torque, &torque);
    
    x_vec3_print(&torque, "Torque");
    x_vec3_print(&pointRelative, "Relative");
}

