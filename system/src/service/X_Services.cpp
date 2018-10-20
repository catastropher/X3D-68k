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

#include "X_ServiceLocator.hpp"
#include "memory/X_LinearAllocator.hpp"
#include "memory/X_Cache.hpp"
#include "memory/X_ZoneAllocator.hpp"
#include "memory/X_MemoryManager.hpp"
#include "filesystem/X_FileHandle.hpp"
#include "filesystem/X_FileSystem.hpp"
#include "platform/X_Platform.hpp"

namespace X3D
{
    static LinearAllocator g_linearAllocator;
    static Cache g_cache;
    static SystemAllocator g_systemAllocator;
    static ZoneAllocator g_zoneAllocator;
    static MemoryManager g_memoryManager;
    
    static FileHandleCache g_fileHandleCache;
    static PakManager g_pakManager;
    static FileSystem g_fileSystem;

    static Platform g_platform;

    template<>
    LinearAllocator* ServiceLocator::get() { return &g_linearAllocator; }

    template<>
    Cache* ServiceLocator::get() { return &g_cache; }

    template<>
    SystemAllocator* ServiceLocator::get() { return &g_systemAllocator; }

    template<>
    ZoneAllocator* ServiceLocator::get() { return &g_zoneAllocator; }

    template<>
    MemoryManager* ServiceLocator::get() { return &g_memoryManager; }

    template<>
    FileHandleCache* ServiceLocator::get() { return &g_fileHandleCache; }

    template<>
    PakManager* ServiceLocator::get() { return &g_pakManager; }

    template<>
    FileSystem* ServiceLocator::get() { return &g_fileSystem; }

    template<>
    Platform* ServiceLocator::get() { return &g_platform; }

    template<>
    ScreenDriver* ServiceLocator::get() { return &g_platform.screenDriver; }
}

