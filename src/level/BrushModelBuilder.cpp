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

#include "BrushModelBuilder.hpp"
#include "BspLevel.hpp"
#include "memory/Memory.hpp"
#include "math/Trig.hpp"
#include "render/ActiveEdge.hpp"

static void buildFace(BspSurface& face, int firstEdgeId, int totalEdges, BspPlane* plane)
{
    for(int i = 0; i < 4; ++i)
    {
        x_cacheentry_init(face.cachedSurfaces + i);
    }
    
    face.faceTexture = nullptr;
    face.firstEdgeId = firstEdgeId;
    face.totalEdges = totalEdges;
    face.flags = (X_BspSurfaceFlags)0;
    face.plane = plane;
    face.color = rand() % 256;
}

void BrushModelBuilder::build()
{
    allocateMemory();
    buildGeometry();
    addBases();
    buildBspTree();
    buildFaces();
    buildEdges();
    
    for(int i = 0; i < totalSurfaces; ++i)
    {
        markSurfaces[i] = &dest.faces[i];
    }
    
    dest.totalFaces = totalSurfaces;
    dest.flags = SURFACE_FILL_SOLID | (255 << 24);
}

void BrushModelBuilder::allocateMemory()
{
    int totalVertices = options.sidesInBase * 2;
    dest.vertices = Zone::alloc<BspVertex>(totalVertices);
    
    int totalEdges = options.sidesInBase * 3 + 1;   // Need an extra edge for edge 0
    dest.edges = Zone::alloc<BspEdge>(totalEdges);
    dest.surfaceEdgeIds = Zone::alloc<int>(options.sidesInBase * 2 + 4 * options.sidesInBase);
    
    dest.faces = Zone::alloc<BspSurface>(totalSurfaces);
    dest.planes = Zone::alloc<BspPlane>(totalSurfaces);
    
    nodes = Zone::alloc<BspNode>(totalSurfaces);
    leaf = Zone::alloc<BspLeaf>();
    
    markSurfaces = Zone::alloc<BspSurface*>(totalSurfaces);
}

void BrushModelBuilder::buildGeometry()
{
    fp angle = 0;
    fp dAngle = fp::fromInt(256) / options.sidesInBase;
    
    prism.vertices = dest.vertices;
    
    // All the points of the prism lie on a circle with some radius because it's a regular polygon.
    // The radius of that circle is sqrt(2) * sideLength
    const fp SQUARE_ROOT_OF_2 = fp::fromFloat(1.414213);
    fp radius = options.sideLength  * SQUARE_ROOT_OF_2 / 2;
    
    // Build the vertices
    for(int i = 0; i < options.sidesInBase; ++i)
    {
        dest.vertices[i].v = Vec3fp(
            radius * x_cos(angle) + options.origin.x,
            -options.height / 2   + options.origin.y,
            radius * x_sin(angle) + options.origin.z);
        
        dest.vertices[i + options.sidesInBase].v = Vec3fp(
            dest.vertices[i].v.x,
            dest.vertices[i].v.y + options.height,
            dest.vertices[i].v.z);
        
        angle += dAngle;
    }
}

void BrushModelBuilder::addBases()
{
    int topVertices[32];
    for(int i = 0; i < options.sidesInBase; ++i)
    {
        topVertices[i] = options.sidesInBase - i - 1;
    }
    
    prism.addFace(topVertices, options.sidesInBase);
    
    int bottomVertices[32];
    for(int i = 0; i < options.sidesInBase; ++i)
    {
        bottomVertices[i] = options.sidesInBase + i;
    }
    
    prism.addFace(bottomVertices, options.sidesInBase);

    int sideVertices[4];
    for(int i = 0; i < options.sidesInBase; ++i)
    {
        int next = (i + 1) % options.sidesInBase;
        sideVertices[0] = i;
        sideVertices[1] = next;
        sideVertices[2] = next + options.sidesInBase;
        sideVertices[3] = i + options.sidesInBase;

        prism.addFace(sideVertices, 4);
    }
}

void BrushModelBuilder::buildBspTree()
{
    for(int i = 0; i < totalSurfaces; ++i)
    {
        nodes[i].frontChild = &nodes[i + 1];
        nodes[i].backChild = (BspNode*)leaf;
        nodes[i].contents = X_BSPLEAF_NODE;
    }
    
    nodes[totalSurfaces - 1].frontChild = (BspNode*)leaf;
    
    leaf->contents = X_BSPLEAF_SOLID;
    
    leaf->firstMarkSurface = markSurfaces;
    leaf->totalMarkSurfaces = totalSurfaces;
    
    dest.rootBspNode = &nodes[0];
}

void BrushModelBuilder::buildEdges()
{
    for(int i = 0; i < prism.totalEdges; ++i)
    {
        dest.edges[i].cachedEdgeOffset = 0;
        dest.edges[i].v[0] = prism.edges[i][0];
        dest.edges[i].v[1] = prism.edges[i][1];
    }
        
    for(int i = 0; i < prism.totalEdgeListIds; ++i)
    {
        dest.surfaceEdgeIds[i] = prism.edgeList[i];
    }
}

void BrushModelBuilder::buildFaces()
{
    for(int i = 0; i < totalSurfaces; ++i)
    {
        dest.planes[i].plane = prism.faces[i].plane;
        buildFace(dest.faces[i], prism.faces[i].edgeListId, prism.faces[i].totalEdges, &dest.planes[i]);
    }
}




