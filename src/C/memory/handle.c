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

#include "X3D_common.h"
#include "X3D_assert.h"
#include "memory/X3D_handle.h"
#include "X3D_enginestate.h"


// Gets the address of where a handle's value is stored
static void** x3d_handle_addr(X3D_Handle handle) {
  return x3d_handlemanager_get()->handles + handle;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new handle that references the given pointer.
///
/// @param ptr  - pointer to reference
///
/// @return The handle ID.
/// @todo Add ability to resize the handle table if out of handles
///////////////////////////////////////////////////////////////////////////////
X3D_Handle x3d_handle_add(void* ptr) {
  X3D_HandleManager* handman = x3d_handlemanager_get();
  
  // Currently can't resize the handle table
  x3d_assert(handman->head);
  
  void** handle = handman->head;
  *handle = ptr;
  
  // Adjust head to the next available handle
  handman->head = *handman->head;
  
  return handle - handman->handles;
}

///////////////////////////////////////////////////////////////////////////////
/// Deletes a handle.
///
/// @param handle - ID of the handle to delete
///
/// @return Nothing.
/// @note No check is made to ensure that the handle actually exists. Very bad
///   things are likely to happen if a handle is deleted twice!
///////////////////////////////////////////////////////////////////////////////
void x3d_handle_delete(X3D_Handle handle) {
  X3D_HandleManager* handman = x3d_handlemanager_get();
  
  // Add the handle to the end of the queue
  void** handle_addr = x3d_handle_addr(handle);
  *handle_addr = NULL;
  
  if(handman->head) {
    *handman->tail = handle_addr;
    handman->tail = handle_addr;
  }
  else {
    handman->head = handle_addr;
    handman->tail = handle_addr;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Dereferences a handle i.e. gets the address referenced by the handle.
///
/// @param handle - handle ID
///
/// @return A pointer to whatever the handle is referencing.
/// @note No check is made to ensure that this is a valid handle!
///////////////////////////////////////////////////////////////////////////////
void* x3d_handle_deref(X3D_Handle handle) {
  return *x3d_handle_addr(handle);
}

///////////////////////////////////////////////////////////////////////////////
/// Initializes the handle manager for the engine.
///
/// @param total_h  - initial number of handles to create
///
/// @return Nothing.
/// @todo Add check that the malloc succeeded.
///////////////////////////////////////////////////////////////////////////////
void x3d_handlemanager_init(uint16 total_h) {
  X3D_HandleManager* handman = x3d_handlemanager_get();
  
  handman->handles = malloc(sizeof(void *) * total_h);
  handman->total_h = total_h;
  
  // Initalize the free list of available handles. This pretty much sets each
  // handle value to point to the next handle value in the table
  uint16 i;
  
  handman->head = handman->handles;
  
  for(i = 0; i < total_h - 1; ++i)
    handman->handles[i] = handman->handles + i + 1;
  
  // Last entry doesn't have a next
  handman->handles[total_h - 1] = NULL;
  handman->tail = handman->handles + total_h - 1;
}

