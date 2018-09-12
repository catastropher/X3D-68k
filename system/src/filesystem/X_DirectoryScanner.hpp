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

#include <dirent.h> 
#include <cstdio> 

#include "X_FilePath.hpp"
#include "X_FileSystemException.hpp"
#include "log/X_Log.hpp"

namespace X3D
{
    class DirectoryScanner
    {
    public:
        DirectoryScanner(const FilePath& path, const char* pattern_ = nullptr)
            : pattern(pattern_)
        {
            directory = opendir(path.c_str());

            if(directory == nullptr)
            {
                Log::error("Failed to open directory %s for scanning", path.c_str());

                throw FileSystemException("Couldn't open directory for scanning");
            }
        }

        bool readEntry(FilePath& dest)
        {
            struct dirent* entry;

            while((entry = readdir(directory)) != nullptr)
            {
                dest.set(entry->d_name);

                if(pattern == nullptr)
                {
                    return true;
                }
                else
                {
                    char extension[FilePath::MAX_LENGTH];
                    dest.getExtension(extension);

                    if(strcmp(extension, pattern) == 0)
                    {
                        return true;
                    }
                }
            }

            return false;
        }


    private:
        DIR* directory;
        const char* pattern;
    };
};

