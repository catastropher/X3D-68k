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

#include "RenderContext.hpp"
#include "Texture.hpp"

struct BspModel;
class X_BspNode;

class WireframeLevelRenderer
{
public:
    WireframeLevelRenderer(X_RenderContext& renderContext_, X_Color levelColor_, X_Color modelColor_)
        : renderContext(renderContext_),
        levelColor(levelColor_),
        modelColor(modelColor_),
        level(*renderContext_.level)
    {
            
    }
    
    void render();
    
private:
    bool edgeHasAlreadyBeenDrawn(int edgeId) const
    {
        return drawnEdges[edgeId / 8] & (1 << (edgeId & 7));
    }
    
    void markEdgeAsAlreadyDrawn(int edgeId)
    {
        drawnEdges[edgeId / 8] |= 1 << (edgeId & 7);
    }
    
    void renderModel(BspModel& model, X_Color color);
    void renderNode(X_BspNode& node, int parentFlags);
    
    X_RenderContext& renderContext;
    X_Color levelColor;
    X_Color modelColor;
    BspLevel& level;
    
    Vec3fp currentModelCenter;
    X_Color currentColor;
    BspModel* currentModel;
    unsigned char drawnEdges[2048];
};

