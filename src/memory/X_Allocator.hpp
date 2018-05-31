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

#include "memory/X_Memory.hpp"

template<typename T>
class XAllocator
{
public:
    using value_type = T;

    value_type* allocate(std::size_t n)
    {
        return Zone::alloc<T>(n);
    }

    void deallocate(value_type* ptr, std::size_t) noexcept
    {
        Zone::free(ptr);
    }
};

template<typename T, typename U>
bool operator==(XAllocator<T> const&, XAllocator<U> const&) noexcept
{
    return true;
}

template <typename T, typename U>
bool operator!=(XAllocator<T> const& x, XAllocator<U> const& y) noexcept
{
    return !(x == y);
}