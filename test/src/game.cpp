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

#include "game.hpp"

void TestGame::shootPortal(Portal* portal)
{
    auto engineContext = getInstance();
    auto level = engineContext->getCurrentLevel();

    Vec3fp camPos = MakeVec3fp(cam->collider.position);

    if(!x_engine_level_is_loaded(engineContext) || level->findLeafPointIsIn(camPos)->contents == X_BSPLEAF_SOLID)
    {
        return;
    }

    int dist = 5000;

    Vec3fp up, forward, right;
    cam->viewMatrix.extractViewVectors(forward, right, up);

    Vec3 f = MakeVec3(forward);

    Vec3 start = cam->collider.position;
    Vec3 end = start + f * dist;

    BoundBox box;

    Ray3 ray(MakeVec3fp(start), MakeVec3fp(end));

    BspRayTracer<X_BspNode*, X_BspNode*> tracer(ray, level, 0);

    portal->poly.constructRegular(16, fp::fromInt(20), 0, Vec3fp(0, 0, 0));

    if(tracer.trace())
    {
        auto& collision = tracer.getCollision();

        // Make sure we're on the normal side of the plane
        if(!collision.plane.pointOnNormalFacingSide(camPos))
        {
            printf("Flip!\n");
            collision.plane.flip();
        }
        else
        {
            //polygon.reverse();
        }

        printf("Hit\n");
        collision.plane.print();
        portal->center = collision.location.point;// + collision.plane.normal * fp::fromFloat(25);
        portal->plane = Plane(collision.plane.normal, portal->center);


        collision.plane.getOrientation(*cam, portal->orientation);

        printf("==========\n");
        portal->orientation.print();
    }

    for(int i = 0; i < portal->poly.totalVertices; ++i)
    {
        portal->poly.vertices[i].y = portal->poly.vertices[i].y * fp::fromFloat(1.75);

        portal->poly.vertices[i] = portal->orientation.transform(portal->poly.vertices[i]) + portal->center;
    }

    //orientation.visualize(Vec3fp(0, 0, 0), *renderContext);

    portal->poly.reverse();

    portal->updatePoly();
}

