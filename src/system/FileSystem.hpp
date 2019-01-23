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

#include <cstdio>

#include "FilePath.hpp"
#include "memory/Link.hpp"

#pragma once

enum FileLocationFlags
{
    IN_PAKFILE = (1 << 0)
};

struct FileLocation
{
    FilePath path;
    FILE* file;
    int flags;
    int pakFileOffset;
};

class FileSystem
{
public:
    static void init(const char* programPath);

    static bool locateFile(const char* name, FileLocation& dest);

    static void addSearchPath(const char* path);

    static FilePath& getProgramPath()
    {
        return searchPathRoot.value;
    }

private:
    static bool locateFileInPakFiles(const char* name, FileLocation& dest);
    static bool locateFileInSearchPaths(const char* name, FileLocation& dest);

    static Link<FilePath> searchPathRoot;
    static Link<FilePath>* pakFileHead;
};

