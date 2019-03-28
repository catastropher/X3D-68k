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

#include "EntityOverlay.hpp"
#include "render/Screen.hpp"
#include "render/RenderingUtil.hpp"
#include "engine/Engine.hpp"
#include "entity/EntityManager.hpp"
#include "entity/builtin/TriggerEntity.hpp"

void EntityOverlay::render()
{
    BoundBoxfp boundBox(
        Vec3fp(-100.0_fp, -100.0_fp, -100.0_fp),
        Vec3fp(100.0_fp, 100.0_fp, 100.0_fp));

    EngineContext* engineContext = Engine::getInstance();

    auto& entities = engineContext->entityManager->getAllEntities();

    Entity* thingWithCamera = nullptr;

    for(Entity* entity : entities)
    {
        if(entity->hasComponent<CameraComponent>())
        {
            thingWithCamera = entity;
            break;
        }
    }

    if(thingWithCamera == nullptr)
    {
        return;
    }

    CameraComponent* cameraComponent = thingWithCamera->getComponent<CameraComponent>();

    X_RenderContext renderContext;
    x_enginecontext_get_rendercontext_for_camera(engineContext, cameraComponent, &renderContext);

    auto& brushModels = engineContext->brushModelSystem->getAllEntities();

    for(Entity* entity : brushModels)
    {
        bool isTrigger = (dynamic_cast<TriggerEntity*>(entity) != nullptr);

        if(!isTrigger)
        {
            continue;
        }

        BspModel* model = entity->getComponent<BrushModelComponent>()->model;

        Vec3fp a = Vec3fp(fp(model->boundBox.v[0].x), fp(model->boundBox.v[0].y), fp(model->boundBox.v[0].z));
        Vec3fp b = Vec3fp(fp(model->boundBox.v[1].x), fp(model->boundBox.v[1].y), fp(model->boundBox.v[1].z));

        BoundBoxfp boundBox(
            a + model->center,
            b + model->center);

        renderBoundBox(boundBox, renderContext, 255);
    }
}
