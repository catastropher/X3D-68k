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

SearchPath FileSystem::root;

void FileSystem::init(const char* programPath)
{
    root
        .set(programPath)
        .removeLastSegment();

    printf("Path: %s\n", root.c_str());

    root.next = nullptr;
}

bool FileSystem::locateFile(const char* name, FilePath& dest)
{
    for(SearchPath* path = &root; path != nullptr; path = path->next)
    {
        dest
            .set(path->c_str())
            .appendSegment(name);

        FILE* file = fopen(dest.c_str(), "rb");
        if(file != nullptr)
        {
            fclose(file);

            return true;
        }
    }

    return false;
}

