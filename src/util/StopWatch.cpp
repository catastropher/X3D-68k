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

#include <sys/time.h>

#include "StopWatch.hpp"
#include "dev/console/Console.hpp"
#include "engine/EngineContext.hpp"

StopWatchEntry StopWatch::entries[X_STOPWATCH_MAX_ENTRIES];
int StopWatch::totalEntries = 0;

static long long getTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    return (long long)1000000 * tv.tv_sec + tv.tv_usec;
    //return clock();
}

void StopWatch::start(const char* name)
{
    auto entry = getEntry(name);

    if(!entry)
    {
        entries[totalEntries].frameTicks = 0;
        entries[totalEntries].totalTicks = 0;
        entries[totalEntries].name = name;
        
        entry = entries + totalEntries++;
    }

    entry->startTick = getTime();
}

void StopWatch::stop(const char* name)
{
    auto entry = getEntry(name);

    if(!entry)
    {
        return;
    }

    entry->frameTicks = getTime() - entry->startTick;
    entry->totalTicks += entry->frameTicks;
}

StopWatchEntry* StopWatch::getEntry(const char* name)
{
    for(int i = 0; i < totalEntries; ++i)
    {
        if(strcmp(name, entries[i].name) == 0)
        {
            return entries + i;
        }
    }

    return nullptr;
}

void StopWatch::stopwatchCmd(X_EngineContext* engineContext, int argc, char* argv[])
{
    StopWatchEntry* total = getEntry("total");

    if(argc == 2 && strcmp(argv[1], "reset") == 0)
    {
        totalEntries = 0;
        return;
    }

    if(total == nullptr)
    {
        x_console_printf(engineContext->getConsole(), "No 'total' entry\n");

        return;
    }

    for(int i = 0; i < totalEntries; ++i)
    {


        x_console_printf(
            engineContext->getConsole(),
            "%s   %.3f%%\n",
            entries[i].name,
            (float)entries[i].totalTicks / total->totalTicks * 100);


    }
}


