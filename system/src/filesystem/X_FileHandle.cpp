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

#define DEBUG_FILE

#include "X_FileHandle.hpp"
#include "X_FileSystemException.hpp"
#include "log/X_Log.hpp"

namespace X3D
{
    void FileHandleCache::init()
    {
        fileHandleCache.fillWithArrayOfNodes(fileHandles, fileHandles + MAX_OPEN_FILE_HANDLES);
    }

    FileHandle* FileHandleCache::openFileForReading(FilePath& path)
    {
        LOG_DEBUG("Try open file %s for reading", path.c_str());

        FileHandle* handle = findUsableHandle(path);

        if(!handle)
        {
            LOG_DEBUG("Couldn't open file %s", path.c_str());
            return nullptr;
        }

        handle->flags.setFlag(FileHandleFlags::inUse);
        fileHandleCache.remove(handle);

        LOG_DEBUG("Opened file %s for reading", path.c_str());

        return handle;
    }

    FileHandle* FileHandleCache::findUsableHandle(FilePath& path)
    {
        for(int i = 0; i < MAX_OPEN_FILE_HANDLES; ++i)
        {
            if(fileHandles[i].canBeReused(path))
            {
                LOG_DEBUG("Found reusable file handle", path.c_str());

                fileHandles[i].reopen();

                return fileHandles + i;
            }
        }

        LOG_DEBUG("Have to hard open file", path.c_str());

        FileHandle* leastRecentlyUsed = fileHandleCache.getLeastRecentlyUsed();

        bool outOfHandles = (leastRecentlyUsed == nullptr);
        if(outOfHandles)
        {
            LOG_DEBUG("Out of file handles\n");

            throw FileSystemException("Out of file handles");
        }

        LOG_DEBUG("Evict handle %d", (int)(leastRecentlyUsed - fileHandles));

        // Out of handles, need to evict whoever was least recently used
        leastRecentlyUsed->forceClose();
        
        return leastRecentlyUsed->open(path)
            ? leastRecentlyUsed
            : nullptr;
    }

    void FileHandleCache::closeFile(FileHandle* handle)
    {
        if(handle->hasAlreadyBeenClosed())
        {
            throw FileSystemException("Trying to close unopened file handle");
        }

        LOG_DEBUG("Closed file %s\n", handle->path.c_str());

        handle->markAsNotInUse();
        fileHandleCache.markAsMostRecentlyUsed(handle);
    }

    void FileHandleCache::cleanup()
    {
        for(int i = 0; i < MAX_OPEN_FILE_HANDLES; ++i)
        {
            fileHandles[i].forceClose();
        }
    }
}

