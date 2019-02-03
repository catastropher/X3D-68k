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
#include "BrushModelComponent.hpp"
#include "BoxColliderComponent.hpp"

#include "level/BrushModelBuilder.hpp"

PlatformEntity::PlatformEntity(X_Edict& edict, BspLevel& level)
    : Entity(level)
{
    //addComponent<BrushModelComponent>(edict, level);

    BrushModelBuilderOptions options;
    options.sideLength = fp::fromInt(50);
    options.height = fp::fromInt(50);

    BspModel* model = new BspModel;
    BrushModelBuilder builder(options, *model);

    builder.build();

    addComponent<BrushModelComponent>(model);
    auto box = addComponent<BoxColliderComponent>();

    box->maxSpeed = fp::fromInt(300);
}

void PlatformEntity::update(X_Time currentTime)
{
    printf("Receive update!\n");
    setNextUpdateTime(currentTime + 500);

    //BrushModelComponent* brushModel = getComponent<BrushModelComponent>();

    //brushModel->model->center.y -= fp::fromInt(10);
}
