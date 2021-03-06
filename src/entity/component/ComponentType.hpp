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

enum class ComponentType
{
    brushModel = (1 << 0),
    transform = (1 << 1),
    camera = (1 << 2),
    input = (1 << 3),
    collider = (1 << 4),
    scriptable = (1 << 5),
    physics = (1 << 6),
    render = (1 << 7)
};