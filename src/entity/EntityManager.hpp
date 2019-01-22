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

#include <vector>
#include <type_traits>

#include "Entity.hpp"
#include "engine/X_Engine.h"

struct X_Edict;

class EntityManager
{
public:
    template<typename TEntity, typename ...ConstructorArgs>
    static TEntity* createEntity(ConstructorArgs&& ...args)
    {
        static_assert(std::is_base_of<Entity, TEntity>::value, "TEntity must derive from Entity");
        
        TEntity* entity = new TEntity(std::forward<ConstructorArgs>(args)...);
        
        int id = entities.size();
        entities.push_back(entity);
        
        entity->id = id;
        
        // FIXME: the level should be a dependency instead of looking it up with the context
        auto context = x_engine_get_context();
        entity->level = x_engine_get_current_level(context);
        
        return entity;
    }
    
    static void updateEntities(X_Time currentTime);
    
    static Entity* createEntityFromEdict(X_Edict& edict);
    
private:
    static std::vector<Entity*> entities;
};

