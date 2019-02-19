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

#include "level/BspLevel.hpp"

class LevelRenderer
{
public:
    LevelRenderer()
        : nextBspKey(0)
    {

    }

    void render(const X_RenderContext& renderContext);

private:
    void renderRecursive(BspNode& node, const X_RenderContext& renderContext, BoundBoxFrustumFlags parentNodeFlags);
    void renderSurfaces(const BspNode& node, const X_RenderContext& renderContext, BoundBoxFrustumFlags geoFlags, fp distanceToPlane);

    static void markAncestorsAsVisible(BspNode& startNode, int currentFrame);
    void renderBrushModels(const X_RenderContext& renderContext);
    void renderBrushModel(BspModel& brushModel, const X_RenderContext& renderContext, BoundBoxFrustumFlags geoFlags);
    static void markSurfacesAsVisible(BspLeaf& leaf, int currentFrame, int leafBspKey);

    int nextBspKey;
};

