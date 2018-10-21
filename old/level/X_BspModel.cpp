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

#include "geo/X_Frustum.h"
#include "level/X_BspLevel.h"
#include "render/X_RenderContext.h"

void X_BspModel::renderWireframe(X_RenderContext& renderContext, X_Color color, unsigned char* drawnEdges)
{
    int flags = (1 << renderContext.viewFrustum->totalPlanes) - 1;

    rootBspNode->renderWireframe(renderContext, color, *this, flags, drawnEdges);
}

