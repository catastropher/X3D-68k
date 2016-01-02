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

typedef enum {
  X3D_RESOURCE_MEM,
  X3D_RESOURCE_FILE,
} X3D_ResourceType;

typedef struct X3D_Resource {
  X3D_ResourceType type;
  void* resource;
} X3D_Resource;

typedef struct X3D_ResourceStack {
  
} X3D_ResourceStack;



void x3d_alloc_mem() {
  
}

void* x3d_transaction_begin() {
  
}

void x3d_transaction_end(void* transaction) {
  
}




void x3d_add_persistant_resource() {
  
}


