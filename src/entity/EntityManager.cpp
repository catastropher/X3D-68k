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

#include "EntityManager.hpp"
#include "object/X_GameObjectLoader.h"  // FIXME
#include "WorldEntity.hpp"
#include "PlatformEntity.hpp"

std::vector<Entity*> EntityManager::entities;

Entity* EntityManager::createEntityFromEdict(X_Edict& edict)
{
    X_EdictAttribute* classname = x_edict_get_attribute(&edict, "classname");
    if(classname == nullptr)
    {
        x_system_error("Edict missing classname");
    }
    
    if(strcmp(classname->value, "worldspawn") == 0)
    {
        return WorldEntity::createFromEdict(edict);
    }
    
    if(strcmp(classname->value, "func_plat") == 0)
    {
        return PlatformEntity::createFromEdict(edict);
    }
    
    return nullptr;
}

void EntityManager::updateEntities(X_Time currentTime)
{
    for(Entity* entity : entities)
    {
        if(entity != nullptr && currentTime >= entity->nextUpdate)
        {
            entity->update(currentTime);
        }
    }
}


