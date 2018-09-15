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

#include "memory/X_FixedLengthString.hpp"
#include "memory/X_Cache.hpp"
#include "memory/X_Array.hpp"
#include "X_FilePath.hpp"
#include "X_FileSearchRequest.hpp"

namespace X3D
{
    class MemoryManager;
    class FileHandleCache;

    struct PakEntry
    {
        FixedLengthString<56> fileName;
        int fileOffset;
        int fileSize;

        static const int FILE_SIZE = 64;
    };

    struct PakHeader
    {
        int fileTableOffset;
        int fileTableSize;
        int totalEntries;
        PakEntry entries[];
    };

    struct PakFile
    {
        char* data;
        int size;
        int fileOffset;
        int pakFileId;
    };

    struct Pak
    {
        FilePath path;
        CacheHandle headerHandle;
        int id;
    };

    class PakManager
    {
    public:
        PakManager(MemoryManager& memoryManager_, FileHandleCache& fileHandleCache_);

        bool readPakFile(FileSearchRequest& request);
        bool locatePakFile(FileSearchRequest& request);

    private:
        void findAllPaks();
        bool searchInPak(Pak& pak, FileSearchRequest& request, PakFile& dest);
        PakHeader* getPakHeader(Pak& pak);

        MemoryManager& memoryManager;
        FileHandleCache& fileHandleCache;

        Array<Pak> paks;
    };
}

