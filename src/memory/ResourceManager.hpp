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

#include <unordered_map>
#include <system/FilePath.hpp>

#include "system/File.hpp"
#include "StringId.hpp"
#include "ResourceHandle.hpp"

class Resource
{
public:
    Resource()
        : referenceCount(0),
        mem(nullptr)
    {

    }

    int referenceCount;
    void* mem;
    FilePath path;
};

class ResourceManager
{
public:
    template<typename TResource>
    ResourceHandle<TResource> getResource(StringId resourceName)
    {
        Resource* resource = getResourceInternal(resourceName);

        if(!resourceIsLoaded(resource))
        {
            resource->mem = loadResource<TResource>(resource->path);
        }

        ++resource->referenceCount;

        return ResourceHandle<TResource>();
    }

    template<typename TResource>
    void releaseResource(ResourceHandle<TResource> handle)
    {
        Resource* resource = handle.resource;

        if(--resource->referenceCount < 0)
        {
            x_system_error("Too many releases for resource");
        }

        if(resource->referenceCount == 0)
        {
            releaseResource<TResource>(resource->mem);
            resource->mem = nullptr;
        }
    }

private:
    Resource* getResourceInternal(StringId resourceName);

    static bool resourceIsLoaded(Resource* resource)
    {
        return resource->mem != nullptr;
    }

    std::unordered_map<int, Resource*> resources;
};