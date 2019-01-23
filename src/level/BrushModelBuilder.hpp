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

#include "math/FixedPoint.hpp"
#include "geo/Vec3.hpp"
#include "level/BspLevel.hpp"

struct BspModel;

struct BrushModelBuilderOptions
{
    int sidesInBase = 4;
    fp sideLength = fp::fromInt(100);
    fp height = fp::fromInt(100);
    Vec3fp origin = Vec3fp(0, 0, 0);
};

struct Prism
{
    Prism()
        : totalEdgeListIds(0),
        totalEdges(1),
        totalFaces(0)
    {
        
    }
    
    struct Face
    {
        int edgeListId;
        int totalEdges;
        Plane plane;
    };
    
    void addFace(int vertexIds[], int totalVertices)
    {
        Face& face = faces[totalFaces++];
        face.totalEdges = totalVertices;
        face.edgeListId = totalEdgeListIds;
        
        for(int i = 0; i < totalVertices; ++i)
        {
            int next = i + 1 < totalVertices
                ? i + 1
                : 0;
                
            edgeList[totalEdgeListIds++] = getOrCreateEdgeBetweenVertices(vertexIds[i], vertexIds[next]);
        }
        
        face.plane = Plane(
            vertices[vertexIds[0]].v,
            vertices[vertexIds[1]].v,
            vertices[vertexIds[2]].v);
    }
        
    int getOrCreateEdgeBetweenVertices(int a, int b)
    {
        for(int j = 1; j < totalEdges; ++j)
        {
            if(edges[j][0] == a && edges[j][1] == b)
            {
                return j;
            }
            else if(edges[j][0] == b && edges[j][1] == a)
            {
                return -j;
            }
        }
        
        edges[totalEdges][0] = a;
        edges[totalEdges][1] = b;
        
        return totalEdges++;
    }
    
    int edgeList[128];
    int totalEdgeListIds;
    
    int edges[128][2];
    int totalEdges;
    
    X_BspVertex* vertices;
    
    Face faces[32];
    int totalFaces;
};

class BrushModelBuilder
{
public:
    BrushModelBuilder(const BrushModelBuilderOptions& options_, BspModel& dest_)
        : options(options_),
        dest(dest_),
        totalSurfaces(options_.sidesInBase + 2)
    {
        
    }
    
    void build();
    
private:
    void allocateMemory();
    void buildGeometry();
    
    void addBases();
    
    void buildBspTree();
    void buildEdges();
    void buildFaces();
    
    const BrushModelBuilderOptions& options;
    BspModel& dest;
    
    const int totalSurfaces;
    
    X_BspLeaf* leaf;
    X_BspNode* nodes;
    X_BspSurface** markSurfaces;
    Prism prism;
};
