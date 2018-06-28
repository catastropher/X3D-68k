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

#define X_FILEPATH_MAX_LENGTH 256

#include <cstring>

class FilePath
{
public:
    FilePath(const char* path_)
    {
        strcpy(path, path_);
    }

    FilePath(const FilePath& filePath)
    {
        strcpy(path, filePath.path);
    }
    
    FilePath()
    {
        path[0] = '\0';
    }

    FilePath& set(const char* path)
    {
        strcpy(this->path, path);

        return *this;
    }

    FilePath& append(const char* str)
    {
        strcat(path, str);

        return *this;
    }

    void getFilename(const char* dest);
    void splitFilename(const char* nameDest, const char* extensionDest);
    FilePath& replaceFilename(const char* newFileName);

    FilePath& removeLastSegment();
    
    FilePath& defaultExtension(const char* defaultExtension);
    FilePath& replaceExtension(const char* newExtension);
    void getExtension(const char* dest);

    FilePath& parentDirectory();

    FilePath& appendSegment(const char* segment);

    FilePath clone();

    const char* c_str() const
    {
        return path;
    }

private:
    char* startOfLastSegment();
    char* end();

    char path[X_FILEPATH_MAX_LENGTH];
};

