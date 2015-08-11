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


// An enum for every function that is to be exported in the DLL. DO NOT
// EDIT THIS DIRECTLY ON YOUR BRANCH! It will lead to different people
// having different export tables in the DLL, which will cause X3D to
// crash really really badly! Instead:
//
// 1) Commit your work on your branch
// 2) Switch to the branch "export-list"
// 3) Do a git pull
// 4) Modify this file on the export-list branch with the new entry in
//      the enum and the new define macro below
// 5) Commit your work and push to "export-list"
// 6) Switch back to your branch
// 7) Merge in the changes from "export-list"
enum {
  EXPORT_X3D_INIT_CORE = 0
};

