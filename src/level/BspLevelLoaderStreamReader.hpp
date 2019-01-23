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

#include "memory/StreamReader.hpp"
#include "BspLevelLoader.hpp"

template<>
StreamReader& StreamReader::read(X_BspLoaderVertex& v)
{
    return readX3dCoord<Vec3f>(v.v);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderFaceTexture& tex)
{
    return readX3dCoord<Vec3f>(tex.uOrientation)
        .readAs<float>(tex.uOffset)
        .readX3dCoord<Vec3f>(tex.vOrientation)
        .readAs<float>(tex.vOffset)
        .read(tex.textureId)
        .read(tex.flags);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderPlane& plane)
{
    return readX3dCoord<Vec3f>(plane.normal)
        .readAs<float>(plane.d)
        .skip<int>();
}

template<>
StreamReader& StreamReader::read(X_BspLoaderFace& face)
{
    return read(face.planeNum)
        .read(face.side)
        .read(face.firstEdge)
        .read(face.totalEdges)
        .read(face.texInfo)
        .readArray(face.lightmapStyles, X_BSPFACE_MAX_LIGHTMAPS)
        .read(face.lightmapOffset);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderLeaf& leaf)
{
    return read(leaf.contents)
        .read(leaf.pvsOffset)
        .readArray(leaf.mins, 3)
        .readArray(leaf.maxs, 3)
        .read(leaf.firstMarkSurface)
        .read(leaf.totalMarkSurfaces)
        .readArray(leaf.ambientLevel, X_BSPLEAF_TOTAL_AMBIENTS);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderNode& node)
{
    return read(node.planeNum)
        .readArray(node.children, 2)
        .readArray(node.mins, 3)
        .readArray(node.maxs, 3)
        .read(node.firstFace)
        .read(node.totalFaces);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderEdge& edge)
{
    return readArray(edge.v, 2);
}

template<>
StreamReader& StreamReader::read(X_BspClipNode& node)
{
    return read(node.planeId)
        .read(node.frontChild)
        .read(node.backChild);
}

template<>
StreamReader& StreamReader::read(X_BspLoaderModel& model)
{
    return readArray(model.mins, 3)
        .readArray(model.maxs, 3)
        .read(model.origin)
        .read(model.rootBspNode)
        .read(model.rootClipNode)
        .read(model.secondRootClipNode)
        .read(model.thirdRootClipNode)
        .read(model.totalBspLeaves)
        .read(model.firstFaceId)
        .read(model.totalFaces);
}

