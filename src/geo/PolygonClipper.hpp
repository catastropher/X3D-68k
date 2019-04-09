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

#include <util/Util.hpp>
#include "Frustum.hpp"
#include "Vec3.hpp"
#include "Plane.hpp"

template<typename TVertex>
const Vec3fp& clipperGetVertex(const TVertex& vertex);

template<typename TVertex>
void clipperClipVertex(const TVertex& start, const TVertex& end, fp t, TVertex& outVertex);


inline const Vec3fp& clipperGetVertex(const Vec3fp& vertex)
{
    return vertex;
}

inline void clipperClipVertex(const Vec3fp& start, const Vec3fp& end, fp t, Vec3fp& outVertex)
{
    outVertex = lerp(start, end, t);
}

template<typename TVertex>
int clipToPlane(TVertex* vertices, int totalVertices, const Plane& plane, TVertex* outVertices)
{
    int totalClipperVertices = 0;

    fp dot = plane.normal.dot(clipperGetVertex(vertices[0]));
    bool in = dot >= -plane.d;

    for(int i = 0; i < totalVertices; ++i)
    {
        int next = (i + 1 < totalVertices ? i + 1 : 0);

        if(in)
        {
            outVertices[totalClipperVertices++] = vertices[i];
        }

        fp nextDot = plane.normal.dot(clipperGetVertex(vertices[next]));
        bool nextIn = nextDot >= -plane.d;
        fp dotDiff = nextDot - dot;

        if(in != nextIn && dotDiff != 0)
        {
            fp scale = (-plane.d - dot) / dotDiff;

            clipperClipVertex(vertices[i], vertices[next], scale, outVertices[totalClipperVertices]);

            ++totalClipperVertices;
        }

        dot = nextDot;
        in = nextIn;
    }

    return totalClipperVertices;
}

#include "error/Error.hpp"

template<typename TVertex>
int clipToFrustum(TVertex* vertices, int totalVertices, const X_Frustum& frustum, TVertex* outVertices, unsigned int clipFlags)
{
    TVertex clippedVertices[2][50];
    int clippedVerticesIndex = 0;
    int lastClipTotalVertices = totalVertices;

    TVertex* verticesToClip = vertices;

    int lastClipPlane = 31 - __builtin_clz(clipFlags);

    for(int plane = 0; plane < lastClipPlane; ++plane)
    {
        bool shouldClipToPlane = (clipFlags & (1 << plane)) != 0;
        if(!shouldClipToPlane)
        {
            continue;
        }

        lastClipTotalVertices = clipToPlane(
            verticesToClip,
            lastClipTotalVertices,
            frustum.planes[plane],
            clippedVertices[clippedVerticesIndex]);

        if(lastClipTotalVertices < 3)
        {
            return 0;
        }

        verticesToClip = clippedVertices[clippedVerticesIndex];
        clippedVerticesIndex ^= 1;
    }

    return clipToPlane(
        verticesToClip,
        lastClipTotalVertices,
        frustum.planes[lastClipPlane],
        outVertices);
}