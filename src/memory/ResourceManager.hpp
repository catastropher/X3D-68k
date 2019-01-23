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

#include "system/File.hpp"

struct Resource;

struct ResourceType
{
    unsigned int id;

    bool (*load)(Resource* resource);
    bool (*unload)(Resource* resource);

    ResourceType* next;
};

struct Resource
{
    char name[32];
    char filename[X_FILENAME_MAX_LENGTH];
    void* mem;

    Resource* next;
};

class ResourceManager
{
    Resource* getResource(const char* name);
};

