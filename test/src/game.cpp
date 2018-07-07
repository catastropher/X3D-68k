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

    X_RayTracer tracer;

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

    x_raytracer_init(&tracer, level, x_bsplevel_get_level_model(level), &start, &end, &box);

    portal->poly.constructRegular(5, fp::fromInt(20), 0, Vec3fp(0, 0, 0));

    if(x_raytracer_trace(&tracer))
    {
        // Make sure we're on the normal side of the plane
        if(!tracer.collisionPlane.pointOnNormalFacingSide(camPos))
        {
            printf("Flip!\n");
            tracer.collisionPlane.flip();
        }
        else
        {
            //polygon.reverse();
        }

        printf("Hit\n");
        tracer.collisionPlane.print();
        portal->center = MakeVec3fp(tracer.collisionPoint);// + tracer.collisionPlane.normal * fp::fromFloat(25);


        tracer.collisionPlane.getOrientation(*cam, portal->orientation);

        

        printf("==========\n");
        portal->orientation.print();
    }

    for(int i = 0; i < portal->poly.totalVertices; ++i)
    {
        portal->poly.vertices[i] = portal->orientation.transform(portal->poly.vertices[i]) + portal->center;
    }

    //orientation.visualize(Vec3fp(0, 0, 0), *renderContext);

    portal->poly.reverse();
}

