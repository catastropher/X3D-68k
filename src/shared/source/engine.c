/* This file is part of X3D.
 *
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */
 
 #include "X3D_engine.h"

uint8 x3d_loaded; 

/// The active X3D context. Do not access directly.
X3D_Context* active_context;

/**
 * Gets the active X3D context.
 *
 * @return The active X3D context.
 *
 * @note Only one X3D context can be active at a time.
 * @note Use this to get the value of @ref active_context.
 */
X3D_Context* x3d_get_active_context() {
  return active_context;
}

/**
 * Sets the active X3D context.
 *
 * @parm context -  context to set the active context to.
 *
 * @return nothing
 *
 * @note Only one X3D context can be active at a time.
 * @note Use this to set the value of @ref active_context.
 */
void x3d_set_active_context(X3D_Context* context) {
  active_context = context;
}

