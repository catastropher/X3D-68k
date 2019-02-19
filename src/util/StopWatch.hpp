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

#define X_STOPWATCH_MAX_ENTRIES 32

struct EngineContext;

struct StopWatchEntry
{
    const char* name;
    long long totalTicks;
    long long frameTicks;

    long long startTick;
};

class StopWatch
{
public:
    static void init()
    {
        totalEntries = 0;
    }

    static void start(const char* name);
    static void stop(const char* name);

    static void print();

    static StopWatchEntry* getEntry(const char* name);

    static void stopwatchCmd(EngineContext* engineContext, int argc, char* argv[]);

    static StopWatchEntry entries[X_STOPWATCH_MAX_ENTRIES];
    static int totalEntries;
};

