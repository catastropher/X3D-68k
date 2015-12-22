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
//
// X3D_config.h -> contains configuration settings for the entire project (this
//    file is specifically for the PC port).


//=============================================================================
// Defines
//=============================================================================

/// The maximum number of memory allocations that can happen simultaneously
#define X3D_MAX_ALLOCS 20

/// The definition line of code for the x3d_engine_state global variable (holds
/// the global state of the engine). This exists because some platforms may 
/// benefit from keeping x3d_engine_state in a register, requiring a different
/// definition/declaration. See also: @ref X3D_ENGINE_STATE_DECL.
#define X3D_ENGINE_STATE_DEFINITION X3D_EngineState* x3d_engine_state

/// The prototype line of code for the x3d_engine_state global variable. See
/// @ref X3D_ENGINE_STATE_DEFINITION for why this exists.
#define X3D_ENGINE_STATE_DECL extern X3D_EngineState* x3d_engine_state