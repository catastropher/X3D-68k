// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_geo.h"
#include "X3D_prism.h"
#include "X3D_render.h"

/**
 * Gets a face polygon from a prism.
 *
 * @param dest    - polygon to save to
 * @param prism   - prism
 * @param face    - face id
 *
 * @return nothing
*/
void x3d_get_prism3d_face(X3D_Polygon3D* dest, X3D_Prism3D* prism, uint16 face) {
  uint16 i;
  uint16 offset = 0;

  if(face == BASE_B)
    offset = prism->base_v;

  if(face == BASE_A || face == BASE_B) {
    for(i = 0; i < prism->base_v; ++i)
      dest->v[i] = prism->v[i + offset];

    dest->total_v = prism->base_v;
  }
  else {
    dest->v[0] = prism->v[face - 2];
    dest->v[1] = prism->v[x3d_single_wrap(face - 2 + 1, prism->base_v)];
    dest->v[2] = prism->v[x3d_single_wrap(face - 2 + 1, prism->base_v) + prism->base_v];
    dest->v[3] = prism->v[face - 2 + prism->base_v];

    dest->total_v = 4;
  }
}

/**
* Sets a face polygon on a prism by replacing the points with the source polygon.
*
* @param src      - source polygon
* @param prism    - prism
* @param face     - f
*
* @return nothing
*/
void x3d_set_prism3d_face(X3D_Polygon3D* src, X3D_Prism3D* prism, uint16 face) {
  uint16 i;
  uint16 offset = 0;

  if(face == BASE_B)
    offset = prism->base_v;

  if(face == BASE_A || face == BASE_B) {
    for(i = 0; i < prism->base_v; ++i)
      prism->v[i + offset] = src->v[i];
  }
  else {
    prism->v[face - 2] = src->v[0];
    prism->v[x3d_single_wrap(face - 2 + 1, prism->base_v)] = src->v[1];
    prism->v[x3d_single_wrap(face - 2 + 1, prism->base_v) + prism->base_v] = src->v[2];
    prism->v[face - 2 + prism->base_v] = src->v[3];
  }
}

