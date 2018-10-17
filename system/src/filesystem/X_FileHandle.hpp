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

#include <cstdio>

#include "memory/X_Lru.hpp"
#include "memory/X_Flags.hpp"
#include "X_FilePath.hpp"

#define MAX_OPEN_FILE_HANDLES 8     // TODO: move to config header

namespace X3D
{
    enum class FileHandleFlags
    {
        inUse = 1,
        forceClose = 2
    };

    class FileHandle : public LruNodeBase<FileHandle>
    {
    public:
        FileHandle()
            : path(""),
            file(nullptr)
        {
            
        }

    private:
        bool canBeReused(FilePath& searchPath)
        {
            return !flags.hasFlag(FileHandleFlags::inUse)
                && path == searchPath;
        }

        void reopen()
        {
            rewind(file);
        }

        void forceClose()
        {
            if(file != nullptr)
            {
                fclose(file);
                file = nullptr;
            }

            flags.clear();
            path.clear();
        }

        void markAsNotInUse()
        {
            flags.clearFlag(FileHandleFlags::inUse);
        }

        bool open(const FilePath& path_)
        {
            file = fopen(path_.c_str(), "rb");

            if(!file)
            {
                return false;
            }

            path = path_;

            return true;
        }

        bool hasAlreadyBeenClosed()
        {
            return next != nullptr
                && prev != nullptr
                && !flags.hasFlag(FileHandleFlags::inUse);
        }

        FilePath path;
        FILE* file;
        Flags<FileHandleFlags> flags;

        friend class FileHandleCache;
        friend class FileReader;
    };

    class FileHandleCache
    {
    public:
        void init();

        FileHandle* openFileForReading(FilePath& path);
        void closeFile(FileHandle* handle);

        void cleanup();

    private:
        FileHandle* findUsableHandle(FilePath& path);

        FileHandle fileHandles[MAX_OPEN_FILE_HANDLES];
        LruList<FileHandle> fileHandleCache;
    };
}