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
            paks[i].path = paths[i];
            paks[i].id = i;
        }
    }

    bool PakManager::locatePakFile(FileSearchRequest& request)
    {
        PakFile dest;
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
        PakHeader* header = getPakHeader(pak);

        for(int i = 0; i < header->totalEntries; ++i)
        {
            FilePath path(header->entries[i].fileName.c_str());

            // We currently only check whether the filenames match
            if(path.matchesFileName(request.path))
            {
                dest.fileOffset = header->entries[i].fileOffset;
                dest.pakFileId = pak.id;
                dest.size = header->entries[i].fileSize;

                return true;
            }
        }

        return false;
    }

    PakHeader* PakManager::getPakHeader(Pak& pak)
    {
        Cache& cache = memoryManager.cache;
        PakHeader* header = (PakHeader*)cache.getCachedData(pak.headerHandle);

        if(header)
        {
            return header;
        }

        FileHandle* handle = fileHandleCache.openFileForReading(pak.path);
    }
}

