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

#include <utility>

namespace X3D
{
    template<typename From, typename To>
    void convert(From& from, To& to);

    // No-op if we're converting to the same type
    template<typename T>
    void convert(T& from, T& to)
    {
        to = from;
    }

    template<typename To, typename From>
    To convert(From from)
    {
        To temp;
        convert(from, temp);

        return std::move(temp);
    }
}

