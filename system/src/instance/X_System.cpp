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

#include "X_System.hpp"
#include "error/X_Exception.hpp"
#include "log/X_Log.hpp"
#include "service/X_ServiceLocator.hpp"
#include "memory/X_MemoryManager.hpp"
#include "filesystem/X_FileSystem.hpp"
#include "platform/X_Platform.hpp"

namespace X3D
{
    void System::init(SystemConfig& config)
    {
        try
        {
            // Init memory services
            initService<LinearAllocator>(config.memoryManager);
            initService<Cache>();
            initService<ZoneAllocator>(config.memoryManager);
            initService<MemoryManager>();

            // Init filesystem
            initService<FileHandleCache>();
            initService<PakManager>();
            initService<FileSystem>();

            // Init platform
            initService<Platform>();
        }
        catch(const Exception& e)
        {
            Log::error(e, "System startup failed");
            throw;
        }
    }

    void System::cleanup()
    {
        // Cleanup platform
        cleanupService<Platform>();
        
        // Zone and cache all allocate from the memory allocated in the linear allocator,
        // so only it needs to be cleaned up
        cleanupService<LinearAllocator>();

        // Cleanup filesystem
        cleanupService<FileHandleCache>();
    }

    // Calls the init() function of the given service type T with the given arguments
    template<typename T, typename ...Args>
    void System::initService(Args&&... args)
    {
        auto service = ServiceLocator::get<T>();
        service->init(std::forward<Args>(args)...);
    }

    template<typename T>
    void System::cleanupService()
    {
        auto service = ServiceLocator::get<T>();
        service->cleanup();
    }
}

