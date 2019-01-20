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

#include "geo/X_Vec3.h"
#include "math/X_Quaternion.h"

#include "Component.hpp"
#include "ComponentDefs.hpp"

namespace internal
{
    class Transform
    {
    public:
        void setPosition(const Vec3fp& position_)
        {
            position = position_;
        }
        
        void setOrientation(const Quaternion& orientation_)
        {
            orientation = orientation_;
        }
        
        void toMat4x4(Mat4x4& dest) const
        {
            Mat4x4 rotation;
            orientation.toMat4x4(rotation);
            
            Mat4x4 translation;
            translation.loadTranslation(position);
            
            dest = translation * rotation;
        }
        
        Vec3fp getPosition()
        {
            return position;
        }
        
    private:
        Vec3fp position;
        Quaternion orientation;
        int standingOnBrushComponent;
    };
}

using TransformComponent = Component<internal::Transform, (int)BuiltinComponents::transform>;

