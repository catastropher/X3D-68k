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
 
 
enum {
  TYPE_INT8,
  TYPE_UINT8,
  TYPE_INT16,
  TYPE_UINT16,
  TYPE_INT32,
  TYPE_UINT32,
  TYPE_VECTOR,
  TYPE_STRING,
  TYPE_ARRAY
};

#define X3D_MAX_VARIABLES 32

typedef struct X3D_Variable {
  char name[16];
  uint16 type;
  void* data;
} X3D_Variable;

typedef struct X3D_VariableManager {
  X3D_Variable var[X3D_MAX_VARIABLES];
} X3D_VariableManager;

#define TYPE(_t) ((_t) & 0xFF)
#define SUBTYPE(_t) ((_t) >> 8)

enum {
  ERROR_NOT_VECTOR,
  ERROR_VAR_NOT_FOUND
};

X3D_Variable* x3d_get_variable(X3D_VariableManager* m, const char* var, uint16* error, char* vector_member) {
  char name[16];
  uint16 i;
  vector_member = ' ';
  char* name_ptr = name;
  
  while(*var) {
    if(*var == '.') {
      char next = var[1];
      
      if(next == 'x' || next == 'y' || next == 'z') {
        if(var[2] == '\0') {
          vector_member = next;
          *name_ptr = '\0';
          break;
        }
      
      }
    }
    
    *name_ptr = var;
    ++name_ptr;
    ++var;
  }
  
  for(i = 0; i < X3D_MAX_VARIABLES; ++i) {
    if(strcmp(m->var[i].name, name) == 0) {
      if(TYPE(m->var[i].type) != TYPE_VECTOR && vector_member != ' ') {
        *error = ERROR_NOT_VECTOR;
        return NULL;
      }
      
      return &m->var[i];
    }
  }
  
  *error = ERROR_VAR_NOT_FOUND;
  return NULL;
}
















