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

#include "geo/X_Vec3.h"

typedef struct X_BspBoundBox
{
    X_Vec3_short min;
    X_Vec3_short max;
} X_BspBoundBox;

typedef struct X_BspModel
{
    
} X_BspModel;

typedef struct X_BspVertex
{
    
} X_BspVertex;

typedef struct X_BspFace
{
    
} X_BspFace;

typedef struct X_BspEdge
{
    
} X_BspEdge;

typedef struct X_BspPlane
{
    
} X_BspPlane;

typedef enum X_BspLeafContents
{
    X_BSPLEAF_NODE = 0,
    X_BSPLEAF_REGULAR = -1,
    X_BSPLEAF_SOLID = -2,
    X_BSPLEAF_WATER = -3,
    X_BSPLEAF_SLIME = -4,
    X_BSPLEAF_LAVA = -5,
    X_BSPLEAF_SKY = -6
} X_BspLeafContents;

struct X_BspNode;

typedef struct X_BspNode
{
    // Common with X_BspLeaf - DO NOT REORDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    X_BspBoundBox boundBox;
    struct X_BspNode* parent;
    
    // Unique elements for node
    X_BspPlane* plane;
    struct X_BspNode* frontChild;
    struct X_BspNode* backChild;
} X_BspNode;

typedef struct X_BspLeaf
{
    // Common with X_BspNode - DO NOT REOREDER
    X_BspLeafContents contents;
    int lastVisibleFrame;
    X_BspBoundBox boundBox;
    struct X_BspNode* parent;
    
    // Unique elements for leaf
    unsigned char* compressedPvsData;
    
} X_BspLeaf;

typedef struct X_BspLevel
{
    X_BspVertex* vertices;
    int totalVertices;
    
    X_BspEdge* edges;
    int totalEdges;
    
    X_BspFace* faces;
    int totalFaces;
    
    X_BspLeaf* leaves;
    int totalLeaves;
    
    X_BspNode* nodes;
    int totalNodes;
    
    X_BspModel* models;
    int totalModels;
} X_BspLevel;

