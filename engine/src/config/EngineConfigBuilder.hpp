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

#include <X3D-System.hpp>

#include "EngineConfig.hpp"

namespace X3D
{
    class EngineConfigBuilder
    {
    public:
        EngineConfigBuilder& memorySize(int linearAllocatorSize_, int zoneAllocatorSize_)
        {
            linearAllocatorSize = linearAllocatorSize_;
            zoneAllocatorSize = zoneAllocatorSize_;

            return *this;
        }

        EngineConfigBuilder& defaultScreenSize()
        {
            screenW = 640;
            screenH = 480;

            return *this;
        }

        void build(EngineConfig& dest);

    private:
        void buildSystem(SystemConfig& config);
        void buildGraphics(GraphicsConfig& config);

        int linearAllocatorSize = 2 * 1024 * 1024;
        int zoneAllocatorSize = 64 * 1024;

        Optional<int> screenW;
        Optional<int> screenH;
    };
}

