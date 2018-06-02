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

#include "X_FileSystem.hpp"

Link<FilePath> FileSystem::searchPathRoot;
Link<FilePath>* FileSystem::pakFileHead;

void FileSystem::init(const char* programPath)
{
    searchPathRoot.value
        .set(programPath)
        .removeLastSegment();

    printf("Path: %s\n", searchPathRoot.value.c_str());

    searchPathRoot.next = nullptr;
}

// Locates a file and opens it for reading
bool FileSystem::locateFile(const char* name, FileLocation& dest)
{
    dest.flags = 0;

    if(locateFileInPakFiles(name, dest))
    {
        return true;
    }

    if(locateFileInSearchPaths(name, dest))
    {
        return true;
    }

    return false;
}

bool FileSystem::locateFileInPakFiles(const char* name, FileLocation& dest)
{
    return false;
}

bool FileSystem::locateFileInSearchPaths(const char* name, FileLocation& dest)
{
    for(auto pathLink = &searchPathRoot; pathLink != nullptr; pathLink = pathLink->next)
    {
        FilePath* path = &pathLink->value;

        dest.path
            .set(path->c_str())
            .appendSegment(name);

        printf("Search %s\n", dest.path.c_str());

        dest.file = fopen(dest.path.c_str(), "rb");
        if(dest.file != nullptr)
        {
            return true;
        }
    }

    return false;
}
