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

#include "BspLevelLoader.hpp"

template<typename T>
const int sizeInFile();

template<>
const int sizeInFile<X_BspClipNode>() { return 8; }

template<>
const int sizeInFile<X_BspLoaderPlane>() { return 20; }

template<>
const int sizeInFile<X_BspLoaderFace>() { return 20; }

template<>
const int sizeInFile<X_BspLoaderLeaf>() { return 28; }

template<>
const int sizeInFile<X_BspLoaderVertex>() { return 12; }

template<>
const int sizeInFile<X_BspLoaderNode>() { return 24; }

template<>
const int sizeInFile<X_BspLoaderEdge>() { return 4; }

template<>
const int sizeInFile<X_BspLoaderModel>() { return 64; }

template<>
const int sizeInFile<unsigned short>() { return 2; }

template<>
const int sizeInFile<unsigned char>() { return 1; }

template<>
const int sizeInFile<int>() { return 4; }

