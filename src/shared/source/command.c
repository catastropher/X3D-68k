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
#include "X3D_segment.h"

void x3d_parse_command(char* str, char** argv, uint16* total) {
  char* arg = str;
  
  *total = 0;
  
  do {
    if(*str == ' ' || *str == '\0') {
      char c = *str;
      
      if(*str) {
        *str = '\0';
        ++str;
      
        while(*str == ' ')
          ++str;
      }
        
      *argv = arg;
      ++argv;
      (*total)++;
      
      if(!c)
        break;
      
      arg = str;
    }
    else {
      ++str;
    }
  } while(1);
}

void x3d_run_command(X3D_Context* context, char** argv, uint16 argc) {
  uint16 i;
  
  if(**argv == 0 || argc == 0)
    return;
    
  if(strcmp(argv[0], "echo") == 0) {
    for(i = 1; i < argc; ++i) {
      printf("'%s'\n", argv[i]);
    }
  }
  else if(strcmp(argv[0], "save") == 0) {
    if(argc == 1 || *argv[1] == '\0') {
      printf("Wrong number of args\n");
    }
    else {
      FILE* file = fopen(argv[1], "wb");
      
      if(file == NULL) {
        printf("Error opening file\n");
      }
      else {
        x3d_save_level(context, file);
        fclose(file);
        
        printf("Level saved\n");
      }
    }
  }
  else if(strcmp(argv[0], "load") == 0) {
    if(argc == 1 || *argv[1] == '\0') {
      printf("Wrong number of args\n");
    }
    else {
      FILE* file = fopen(argv[1], "rb");
      
      if(file == NULL) {
        printf("Error opening file\n");
      }
      else {
        x3d_load_level(context, file);
        fclose(file);
        
        printf("Clearing objects...\n");
        x3d_clear_all_objects(context);
        
        printf("Creating camera\n");
        X3D_Camera* cam = (X3D_Camera*)x3d_create_object(context, 0, (Vex3D){ 0, 0, 0 }, (Vex3D_angle256){ 0, 0, 0 }, (Vex3D_fp0x16){ 0, 0, 0 }, FALSE, 0);
        cam->object.pos = (Vex3D_fp16x16){ 0, 0, 0 };
        cam->object.angle = (Vex3D_angle256){ 0, 0, 0 };
        
        context->cam = cam;
        
        printf("Level loaded\n");
      }
    }
  }
  else if(strcmp(argv[0], "size") == 0) {
    if(argc > 1) {
      printf("%d\n", x3d_segment_needed_size(atoi(argv[1])));
    }
  }
  else {
    printf("Unknown command\n");
  }

}
 
void x3d_enter_console(X3D_Context* context) {
  char buf[256];
  char* args[32];
  uint16 argc;
  
  INT_HANDLER old_int_1 = GetIntVec(AUTO_INT_1);
  INT_HANDLER old_int_5 = GetIntVec(AUTO_INT_5);
  
  // Reset the int handlers
  SetIntVec(AUTO_INT_1, context->old_int_1);
  SetIntVec(AUTO_INT_5, context->old_int_5);
  
  // Reset the screen
  PortRestore();
  
  clrscr();
  
  printf("X3D console\nBuild date: %s %s\n", __DATE__, __TIME__);
  
  do {
    printf(">");
    gets(buf);
    
    if(strcmp("done", buf) == 0)
      break;
    
    printf("\n");
    x3d_parse_command(buf, args, &argc);
    x3d_run_command(context, args, argc);
  } while(1);
  
  
  // Restore the screen
  PortSet(context->screen_data, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  
  // Restore the int handlers
  SetIntVec(AUTO_INT_1, old_int_1);
  SetIntVec(AUTO_INT_5, old_int_5);
  
  while(_keytest(RR_ENTER)) ;
}

