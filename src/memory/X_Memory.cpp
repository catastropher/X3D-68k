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

#include <cstdio>
#include <cstring>

#include "X_Memory.hpp"
#include "error/X_error.h"
#include "error/X_log.h"
#include "util/X_util.h"
#include "memory/X_String.h"
#include "engine/X_init.h"
#include "util/X_Json.hpp"

unsigned char* Hunk::highMark;
unsigned char* Hunk::lowMark;

Zone::Block* Zone::rover;

void Cache::freeBelow(void* ptr)
{
}

void Cache::freeAbove(void* ptr)
{
}

void MemoryManager::init(int hunkSize, int zoneSize)
{
    Log::info("Initializing memory manager");

    Hunk::init(hunkSize);
    Zone::init(zoneSize);
    Json::init();
}

void MemoryManager::cleanup()
{
    // Only the hunk needs to be cleaned up because the other allocators allocate
    // space from the hunk
    Hunk::cleanup();
}

