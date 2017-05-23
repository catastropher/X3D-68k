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

#define X_SWAP(_a, _b) { __typeof__(_a) _temp = (_a); (_a) = (_b); (_b) = _temp; }

#define X_MIN(_a, _b) ({ __typeof__(_a) _aa = _a; __typeof__(_b) _bb = _b; _aa < _bb ? _aa : _bb; })
#define X_MAX(_a, _b) ({ __typeof__(_a) _aa = _a; __typeof__(_b) _bb = _b; _aa > _bb ? _aa : _bb; })

#define X_SIGNOF(_v) ((_v) < 0 ? -1 : ((_v) > 0 ? 1 : 0))

