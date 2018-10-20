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

#include <SDL/SDL.h>

#include "X_BasePlatform.hpp"
#include "screen/X_PcScreenDriver.hpp"
#include "log/X_Log.hpp"

namespace X3D
{
    class PcPlatform : public BasePlatform<PcPlatform, PcScreenDriver>
    {
    public:
        void init()
        {
            Log::info("Init PC");

            initModules();
        }

        void cleanup()
        {
            cleanupModules();

            SDL_Quit();
        }

        void update()
        {
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                
            }
        }
    };
}

