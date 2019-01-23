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

#include "ActiveEdgeRenderer.hpp"
#include "object/CameraObject.hpp"

static bool edgeIsReversed(int bspEdgeId)
{
    return bspEdgeId < 0;
}

Vertex* ActiveEdgeRenderer::getCreateCachedVertex(Vec3* v, int bspVertexId)
{
    X_BspVertex* bspVertex = level->vertices + bspVertexId;
    Vertex* vertex = (Vertex*)((unsigned char*)vertices.begin() + bspVertex->cacheOffset);
    
    if(vertex->bspVertedId == bspVertexId && vertex->lastFrame == currentFrame)
        return vertex;
    
    Vertex* newVertex = vertices.alloc();
    newVertex->bspVertedId = bspVertexId;
    newVertex->lastFrame = currentFrame;
    newVertex->next = nullptr;
    
    x_viewport_project_vec3(&renderContext->cam->viewport, v, &newVertex->projected);
    
    if(bspVertexId != -1)
        bspVertex->cacheOffset = (unsigned char*)newVertex - (unsigned char*)vertices.begin();
    
    return newVertex;
}

void ActiveEdgeRenderer::addEdge(Vec3* a, Vec3* b, int bspEdgeId, Surface* surface)
{
    
}

