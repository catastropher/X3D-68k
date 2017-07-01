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

#include "X_Texture.h"
#include "system/X_File.h"
#include "error/X_log.h"

_Bool x_texture_save_to_xtex_file(const X_Texture* tex, const char* fileName)
{
    X_File file;
    if(!x_file_open_writing(&file, fileName))
        return 0;
    
    x_file_write_buf(&file, 4, "XTEX");
    x_file_write_le_int16(&file, x_texture_w(tex));
    x_file_write_le_int16(&file, x_texture_h(tex));
    x_file_write_buf(&file, x_texture_total_texels(tex), tex->texels);
    
    x_file_close(&file);
    
    return 1;
}

_Bool x_texture_load_from_xtex_file(X_Texture* tex, const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return 0;
    
    char signature[5];
    x_file_read_fixed_length_str(&file, 4, signature);
    
    if(strcmp(signature, "XTEX") != 0)
    {
        x_log_error("File %s has bad XTEX header", fileName);
        return 0;
    }
    
    int w = x_file_read_le_int16(&file);
    int h = x_file_read_le_int16(&file);
    
    x_texture_init(tex, w, h);
    x_file_read_buf(&file, x_texture_total_texels(tex), tex->texels);
    
    x_file_close(&file);
    
    return 1;
}

