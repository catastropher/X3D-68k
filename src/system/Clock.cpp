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

#include <unistd.h>

#include "Clock.hpp"
#include "engine/Config.hpp"

int Clock::lastRecordedClock;
int Clock::currentTick;

Time Clock::getTicks()
{
    int tick;
    
#ifndef X_GET_TIME_USING_SDL
    tick = clock() * 1000 / CLOCKS_PER_SEC;
#else
    tick = SDL_GetTicks();
#endif
    
    currentTick += (tick - lastRecordedClock);
    lastRecordedClock = tick;
    
    return Time::fromMilliseconds(currentTick);
}

void Clock::delay(Duration duration)
{
    int milliseconds = duration.toMilliseconds();

    currentTick += milliseconds;
    
    usleep(milliseconds * 1000);
}


