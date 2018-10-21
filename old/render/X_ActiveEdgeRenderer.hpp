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

#include "math/X_fix.h"
#include "memory/X_ArenaAllocator.hpp"
#include "geo/X_Vec3.h"
#include "level/X_BspLevel.h"
#include "render/X_RenderContext.h"

struct Surface;
struct Edge;

struct EdgeSurfacePair
{
    Surface* surface;
    Edge* edge;
    EdgeSurfacePair* next;
};

struct Vertex
{
    X_Vec2 projected;
    int lastFrame;
    int bspVertedId;
    EdgeSurfacePair firstPair;
    
    Vertex* next;
};

template<typename T>
struct Link
{
    void insertAfter(T* node) { }
    void insertBefore(T* node) { }
    void unlink() { }
    
    T* next;
    T* prev;
};

struct Surface
{
    Surface* parent;
    
    Surface* getParent() 
    {
        if(parent == this)
            return this;
        
        parent = parent->getParent();
        return parent;
    }
    
    void unionSurface(Surface* surface)
    {
        surface->getParent()->parent = this->getParent();
    }
};

struct Edge : Link<Edge>
{    
    fp getX(int y)
    {
        return y * slope + xIntercept;
    }
    
    fp xIntercept;
    fp slope;
    Surface* surface;
};

struct SilhouetteEdge : Link<SilhouetteEdge>
{
    Edge* edge;
    Surface* parent;
};

class ActiveEdgeRenderer
{
public:
    ActiveEdgeRenderer()
        : edges(1000, "EdgeArena"),
        vertices(1000, "VertexArena"),
        surfaces(1000, "SurfaceArena"),
        pairs(1000, "PairArena")
    {
        
    }
    
    void addEdge(Vec3* a, Vec3* b, int bspEdgeId);
    
private:
    ArenaAllocator<Edge> edges;
    ArenaAllocator<Vertex> vertices;
    ArenaAllocator<Surface> surfaces;
    ArenaAllocator<EdgeSurfacePair> pairs;
    
    X_BspLevel* level;
    int currentFrame;
    
    X_RenderContext* renderContext;
};

