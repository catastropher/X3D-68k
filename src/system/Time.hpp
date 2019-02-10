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

#include "math/FixedPoint.hpp"

// 1024 ticks per second
class Duration
{
public:
    constexpr Duration()
        : ticks(0)
    {

    }

    constexpr Duration operator+(const Duration& rhs) const
    {
        return Duration(ticks + rhs.ticks);
    }

    constexpr Duration operator-(const Duration& rhs) const
    {
        return Duration(ticks - rhs.ticks);
    }

    constexpr Duration operator*(int multiplier) const
    {
        return Duration(ticks * multiplier);
    }

    constexpr Duration operator/(int divisor) const
    {
        return Duration(ticks / divisor);
    }

    constexpr fp operator/(const Duration& rhs) const
    {
        return fp((ticks << 10) / rhs.ticks * 64);
    }

    constexpr fp toSeconds() const
    {
        return fp(ticks << 6);
    }

    constexpr int toTicks() const
    {
        return ticks;
    }

    constexpr int toMilliseconds() const
    {
        return ticks * 1000 / 1024;
    }

    constexpr bool operator<(const Duration& rhs) const
    {
        return ticks < rhs.ticks;
    }

    constexpr bool operator<=(const Duration& rhs) const
    {
        return ticks < rhs.ticks;
    }

    constexpr bool operator>(const Duration& rhs) const
    {
        return ticks > rhs.ticks;
    }

    constexpr bool operator>=(const Duration& rhs) const
    {
        return ticks >= rhs.ticks;
    }

    constexpr bool operator==(const Duration& rhs) const
    {
        return ticks == rhs.ticks;
    }

    constexpr bool operator!=(const Duration& rhs) const
    {
        return ticks != rhs.ticks;
    }

    static constexpr Duration fromMilliseconds(int milliseconds)
    {
        return Duration(milliseconds * 1024 / 1000);
    }

    static constexpr Duration fromSeconds(fp seconds)
    {
        // Shift from 2^16 base down to 2^10
        return Duration(seconds.asRightShiftedInteger(6));
    }

    static constexpr Duration fromMinutes(fp minutes)
    {
        return Duration(fromSeconds(minutes * 60));
    }

    static constexpr Duration fromHours(fp hours)
    {
        return Duration(fromMinutes(hours * 60));
    }

    static constexpr Duration fromTicks(int ticks)
    {
        return Duration(ticks);
    }

private:
    constexpr Duration(int ticks_)
        : ticks(ticks_)
    {

    }

    int ticks;
};

class Time
{
public:
    constexpr Time()
        : ticksFromProgramStart(0)
    {

    }

    static constexpr Time fromMilliseconds(int milliseconds)
    {
        return Time(milliseconds * 1024 / 1000);
    }

    Duration operator-(const Time& rhs) const
    {
        return Duration::fromTicks(ticksFromProgramStart - rhs.ticksFromProgramStart);
    }

    Time operator+(const Duration& duration) const
    {
        return Time(ticksFromProgramStart + duration.toTicks());
    }

    Time operator-(const Duration& duration) const
    {
        return Time(ticksFromProgramStart - duration.toTicks());
    }

    constexpr bool operator<(const Time& rhs) const
    {
        return ticksFromProgramStart < rhs.ticksFromProgramStart;
    }

    constexpr bool operator<=(const Time& rhs) const
    {
        return ticksFromProgramStart <= rhs.ticksFromProgramStart;
    }

    constexpr bool operator>(const Time& rhs) const
    {
        return ticksFromProgramStart > rhs.ticksFromProgramStart;
    }

    constexpr bool operator>=(const Time& rhs) const
    {
        return ticksFromProgramStart >= rhs.ticksFromProgramStart;
    }

    constexpr bool operator==(const Time& rhs) const
    {
        return ticksFromProgramStart == rhs.ticksFromProgramStart;
    }

    constexpr bool operator!=(const Time& rhs) const
    {
        return ticksFromProgramStart != rhs.ticksFromProgramStart;
    }

private:
    constexpr Time(int ticksFromProgramStart)
        : ticksFromProgramStart(ticksFromProgramStart)
    {

    }

    int ticksFromProgramStart;
};


