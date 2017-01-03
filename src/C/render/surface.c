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

#include "render/X3D_surface.h"
#include "X3D_trig.h"

void x3d_surface_apply_primary_texture(X3D_Surface* surface, X3D_Texture* tex, X3D_TextureOrientation orientation) {
    X3D_Vex2D tex_coordinate_system = x3d_vex2d_make(x3d_cos(orientation.angle), x3d_sin(orientation.angle));
}

