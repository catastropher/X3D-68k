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

#include "PlatformEntity.hpp"
#include "EntityManager.hpp"
#include "object/GameObjectLoader.hpp"

#include "level/BrushModelBuilder.hpp"

PlatformEntity* PlatformEntity::createFromEdict(X_Edict& edict)
{
    auto platform =  EntityManager::createEntity<PlatformEntity>();
    auto brushModel = platform->getComponent<BrushModelComponent>();
    
    brushModel->initFromEdict(edict, platform->getLevel());
    
    BrushModelBuilderOptions options;
    
    options.sidesInBase = 5;
    
    options.origin = Vec3fp(
        fp::fromFloat(159.377563),
        fp::fromFloat(64.371002),
        fp::fromFloat(-822.340271));
    
    BrushModelBuilder builder(options, *brushModel->model);
    
    //builder.build();
    
    return platform;
}

void PlatformEntity::update(X_Time currentTime)
{
    printf("Receive update!\n");
    setNextUpdateTime(currentTime + 500);

    BrushModelComponent* brushModel = getComponent<BrushModelComponent>();

    //brushModel->model->center.y -= fp::fromInt(10);
}
