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

#include <X3D-System.hpp>

#include "Screen.hpp"
#include "LightingTable.hpp"

namespace X3D
{
    static Screen g_screen;
    static LightingTable g_lightingTable;

    template<>
    Screen* ServiceLocator::get()
    {
        return &g_screen;
    }

    template<>
    LightingTable* ServiceLocator::get()
    {
        return & g_lightingTable;
    }
}

