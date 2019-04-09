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

#include <system/Time.hpp>
#include "level/EntityModel.hpp"

enum class RenderComponentType
{
    quake,
    brush,
    billboard
};

class RenderComponent
{
public:
    RenderComponent(RenderComponentType type_)
        : type(type_)
    {

    }

    RenderComponentType type;
};

class QuakeModelRenderComponent : public RenderComponent
{
public:
    QuakeModelRenderComponent()
        : RenderComponent(RenderComponentType::quake),
        currentFrame(nullptr),
        playingAnimation(false)
    {

    }

    void playAnimation(const char* animationName, bool loop = false);

    X_EntityModel* model;
    X_EntityFrame* currentFrame;
    X_EntityFrame* animationStartFrame;
    Time frameStart;
    bool playingAnimation;
    bool loopAnimation;
};

class BillboardRenderComponent : public RenderComponent
{
public:
    BillboardRenderComponent()
        : RenderComponent(RenderComponentType::billboard)
    {

    }

    Texture* texture;
};

