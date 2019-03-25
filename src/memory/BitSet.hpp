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

template<typename T>
class Flags
{
public:
    constexpr Flags()
        : mask(0)
    {
        
    }

    template<typename ...InitialFlags>
    explicit constexpr Flags(InitialFlags... initialFlags)
        : mask(buildMask(std::forward<InitialFlags>(initialFlags)...))
    {

    }

    constexpr Flags(unsigned int flags)
        : mask(0)
    {

    }

    constexpr void set(T flag)
    {
        mask |= (int)flag;
    }

    constexpr void set(unsigned int mask)
    {
        this->mask = mask;
    }

    constexpr void reset(T flag)
    {
        mask &= ~flag;
    }

    constexpr bool hasFlag(T flag) const
    {
        return (mask & (int)flag) != 0;
    }

    constexpr void clear()
    {
        mask = 0;
    }

    constexpr unsigned int getMask() const
    {
        return mask;
    }

private:
    constexpr unsigned int buildMask(T value)
    {
        return (unsigned int)value;
    }

    template<typename ...Ts>
    constexpr int buildMask(T value, Ts... rest)
    {
        return (unsigned int)value | buildMask(rest...);
    }

    unsigned int mask;
};

