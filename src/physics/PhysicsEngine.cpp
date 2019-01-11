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

#include "PhysicsEngine.hpp"
#include "entity/BoxColliderComponent.hpp"
#include "X_BoxColliderEngine.hpp"

// FIXME
extern bool physics;

X_Time PhysicsEngine::lastUpdate;

void PhysicsEngine::update(BspLevel& level, X_Time currentTime)
{
    const int TIME_PER_STEP = 1000 / UPDATE_FREQUENCY;
    int updateCount = 0;
    
    while(lastUpdate + TIME_PER_STEP <= currentTime)
    {
        step(level);
        lastUpdate += TIME_PER_STEP;
        ++updateCount;
    }
    
    auto boxColliders = BoxColliderComponent::getAll();
    
    for(auto& collider : boxColliders)
    {
        collider.frameVelocity = Vec3fp(0, 0, 0);
    }
}


void PhysicsEngine::step(BspLevel& level)
{
    if(!physics)
    {
        return;
    }
    
    auto boxColliders = BoxColliderComponent::getAll();
    
    for(auto& collider : boxColliders)
    {
        collider.velocity += collider.impulseVelocity + collider.frameVelocity;
        
        BoxColliderEngine engine(collider, level);
        engine.runStep();
        
        collider.impulseVelocity = Vec3fp(0, 0, 0);
    }
}


