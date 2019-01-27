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

#include "EngineContext.hpp"

#define X_MAJOR_VERSION 0
#define X_MINOR_VERSION 1
#define X_VERSION (X_MAJOR_VERSION * 1000 + X_MINOR_VERSION)

class Engine
{
public:
    static X_EngineContext* init(X_Config& config);
    static void quit();
    static void run();

    X_EngineContext* getInstance()
    {
        return &instance;
    }

private:
    static void shutdownEngine();

    static X_EngineContext instance;
    static bool wasInitialized;
    static bool isDone;
};

