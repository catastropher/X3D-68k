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
    class FileHandle;

    struct PakEntry
    {
        FixedLengthString<56> fileName;
        int fileOffset;
        int fileSize;

        static const int FILE_SIZE = 64;
    };

    struct PakFileTable
    {
        int totalEntries;
        PakEntry entries[];
    };

    struct PakFile
    {
        Array<char> data;
        int fileOffset;
        int pakFileId;
    };

    struct Pak
    {
        FilePath path;
        CacheHandle headerHandle;
        int id;
        int fileTableOffset;
        int fileTableSize;
        FileHandle* fileHandle;

        static const unsigned int MAGIC_NUMBER = (('P') + ('A' << 8) + ('C' << 16) + ('K' << 24));
    };

    class PakManager
    {
    public:
        PakManager(MemoryManager& memoryManager_, FileHandleCache& fileHandleCache_);

        bool readPakFile(FileSearchRequest& request, PakFile& dest);
        bool locatePakFile(FileSearchRequest& request, PakFile& dest);

    private:
        void findAllPaks();
        void readPakHeader(Pak& pak);

        void openPak(Pak& pak);
        void closePak(Pak& pak);

        bool searchInPak(Pak& pak, FileSearchRequest& request, PakFile& dest);
        PakFileTable* getFileTable(Pak& pak);
        PakFileTable* loadFileTable(Pak& pak);

        MemoryManager& memoryManager;
        FileHandleCache& fileHandleCache;

        Array<Pak> paks;
    };
}

