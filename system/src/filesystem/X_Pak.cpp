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

#include "X_Pak.hpp"
#include "X_DirectoryScanner.hpp"
#include "memory/X_MemoryManager.hpp"
#include "X_FileHandle.hpp"
#include "memory/X_StreamReader.hpp"
#include "memory/X_Array.hpp"
#include "X_FileReader.hpp"

namespace X3D
{
    PakManager::PakManager(MemoryManager& memoryManager_, FileHandleCache& fileHandleCache_)
        : memoryManager(memoryManager_),
        fileHandleCache(fileHandleCache_)
    {
        findAllPaks();
    }

    void PakManager::findAllPaks()
    {
        const int MAX_PAKS = 20;

        int totalPaks = 0;
        FilePath paths[MAX_PAKS];

        DirectoryScanner scanner(".", "pak");

        Log::info("Scanning for PAK files...");

        while(scanner.readEntry(paths[totalPaks]))
        {
            Log::info("\t\t - Found PAK file %s", paths[totalPaks].c_str());
            ++totalPaks;
        }

        Log::info("Found %d PAK files", totalPaks);

        Pak* paksMem = (Pak*)memoryManager.linearAllocator.allocStable(sizeof(Pak) * totalPaks, "pakfiles");
        paks.set(paksMem, totalPaks);

        for(int i = 0; i < totalPaks; ++i)
        {
            paks[i].fileHandle = nullptr;
            paks[i].path = paths[i];
            paks[i].id = i;

            readPakHeader(paks[i]);
        }
    }

    void PakManager::readPakHeader(Pak& pak)
    {
        Log::info("Read PAK header of %s\n", pak.path.c_str());

        openPak(pak);
        FileReader fileReader(pak.fileHandle);

        const int HEADER_SIZE = 12;
        FixedSizeArray<char, HEADER_SIZE> data;

        fileReader.read(data, HEADER_SIZE);
        closePak(pak);

        StreamReader reader(data);

        unsigned int fileMagicNumber;
        reader
            .read(fileMagicNumber)
            .read(pak.fileTableOffset)
            .read(pak.fileTableSize);

        const unsigned int PAK_MAGIC_NUMBER = (('P') + ('A' << 8) + ('C' << 16) + ('K' << 24));

        if(fileMagicNumber != PAK_MAGIC_NUMBER)
        {
            throw FileSystemException("Invalid PAK file");
        }
    }

    void PakManager::openPak(Pak& pak)
    {
        FileHandle* handle = fileHandleCache.openFileForReading(pak.path);

        if(!handle)
        {
            throw FileSystemException("Failed to open PAK file");
        }

        pak.fileHandle = handle;
    }

    void PakManager::closePak(Pak& pak)
    {
        fileHandleCache.closeFile(pak.fileHandle);
    }

    bool PakManager::readPakFile(FileSearchRequest& request, PakFile& dest)
    {
        if(!locatePakFile(request, dest))
        {
            return false;
        }

        Pak& pak = paks[dest.pakFileId];
        openPak(pak);

        memoryManager.alloc(dest.data.size, request.source, dest.data);

        FileReader reader(pak.fileHandle);
        reader.seek(dest.fileOffset);
        reader.read(dest.data, dest.data.size);

        return true;
    }

    bool PakManager::locatePakFile(FileSearchRequest& request, PakFile& dest)
    {
        for(Pak& pak : paks)
        {
            if(searchInPak(pak, request, dest))
            {
                return true;
            }
        }

        return false;
    }

    bool PakManager::searchInPak(Pak& pak, FileSearchRequest& request, PakFile& dest)
    {
        PakFileTable* header = getFileTable(pak);

        for(int i = 0; i < header->totalEntries; ++i)
        {
            FilePath path(header->entries[i].fileName.c_str());

            // We currently only check whether the filenames match
            if(path.matchesFileName(request.path))
            {
                dest.fileOffset = header->entries[i].fileOffset;
                dest.pakFileId = pak.id;
                dest.data.size = header->entries[i].fileSize;
                dest.data.data = nullptr;

                return true;
            }
        }

        return false;
    }

    PakFileTable* PakManager::getFileTable(Pak& pak)
    {
        Cache& cache = memoryManager.cache;

        return pak.headerHandle.dataInCache()
            ? (PakFileTable*)cache.getCachedData(pak.headerHandle)
            : loadFileTable(pak);
    }

    PakFileTable* PakManager::loadFileTable(Pak& pak)
    {
        Cache& cache = memoryManager.cache;

        cache.alloc(
            sizeof(PakFileTable) + pak.fileTableSize,
            pak.headerHandle,
            0);

        PakFileTable* fileTable = (PakFileTable*)cache.getCachedData(pak.headerHandle);

        fileTable->totalEntries = pak.fileTableSize / PakEntry::FILE_SIZE;

        // This reads the header in place...
        // TODO: make safe for little endian systems
        Array<char> fileTableData(
            (char*)fileTable->entries,
            pak.fileTableSize);

        openPak(pak);
        FileReader reader(pak.fileHandle);

        reader.seek(pak.fileTableOffset);
        reader.read(fileTableData, pak.fileTableSize);

        closePak(pak);

        return fileTable;
    }
}

