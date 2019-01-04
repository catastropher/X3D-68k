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

#include "BrushModelComponent.hpp"
#include "object/X_GameObjectLoader.h"

namespace internal
{
    void BrushModel::initFromEdict(X_Edict& edict, BspLevel& level)
    {
        int modelId = x_edict_get_model_id(&edict, "model");
        
        model = x_bsplevel_get_model(&level, modelId);
    }
}

