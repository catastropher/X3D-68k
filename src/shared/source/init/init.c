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

#include "X3D_config.h"
#include "init/X3D_init.h"
#include "X3D_engine.h"

/**
 * Initializes logging.
 *
 * @param context - context to set up logging for
 * @param flags   - logging flags 
 *
 * @note Do not call this directly. It is called from x3d_init_core().
 */
static inline x3d_init_log(X3D_Context* context, uint8 flags) {
  context->log.flags = flags;
  
  if((context->log.flags & X3D_ENABLE_LOG_FILE) != 0) {
    context->log.file = fopen("log", "wt");
    
    if(context->log.file == NULL) {
      context->log.flags &= ~X3D_ENABLE_LOG_FILE;
      x3d_log(X3D_ERROR, "Failed to open log file\n");
    }
  }
  
  x3d_log(X3D_INFO, "Init log\n");
}

/**
 * Initializes the core engine.
 *
 * @param context - context to set as active context
 * @param config  - configuration settings
 *
 * @note Do not call this directly. It is called from x3d_init().
 */
uint16 x3d_init_core(X3D_Context* context, X3D_Config* config) {
  clrscr();
  
  x3d_set_active_context(context);
  x3d_init_log(context, config->log_flags);
  
  return 0;
}

/**
 * Cleans up the engine core and releases any resources.
 *
 * @note Do not call this directly. It is called from x3d_cleanup().
 * @todo Should this be moved into a separate file?
 */
void x3d_cleanup_core(void) {
  X3D_Context* context = x3d_get_active_context();
  
  
  if((context->log.flags & X3D_ENABLE_LOG_FILE) != 0) {
    
    if(fclose(context->log.file) == EOF) {
    }
  }
}
