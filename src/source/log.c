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

#include "X3D_config.h"
#include "X3D_fix.h"


static _Bool parse_int(const char** str, int16* i) {
  int16 res = 0;

  while(isdigit(**str)) {
    res = (res * 10) + **str - '0';
    (*str)++;
  }

  *i = res;

  return 1;
}

/**
 * Determines if a string denotes a fixed point number format. The format expected
 * is fp[W].[F] where W is the number of whole bits and F is the number of
 * fractional bits.
 * 
 * @param str    - pointer to a c string pointer that contains the string to parse
 * @param whole  - pointer to where the number of whole bits should be written
 *   (if it's actually an fp)
 *
 * @param frac   - pointer to where the number of frac bits should be written
 *   (if it's actually an fp)
 *
 * @return Whether the string is in fixed point format
 * @note This advances *str if it matches the format.
 * @note For internal use only.
 */
static _Bool parse_fp(const char** str, int16* whole, int16* frac) {
  if(**str == 'f' && *(*str + 1) == 'p') {
    *str += 2;

    if(parse_int(str, whole)) {
      if(**str == '.') {
        ++(*str);

        if(parse_int(str, frac)) {
          return 1;
        }
      }
    }
  }

  return 0;
}


/**
 * Sends formated output to a string. x3d_sprintf is somewhat like sprintf, but
 * its syntax is different, as well as what types it can print.
 *
 * Supported types:<ul>
 *   <li><code>\@i[B]</code> - an integer with B bits. Example: <code>\@i32</code></li>
 *   <li><code>\@s</code> - a string</li>
 *   <li><code>\@d[B]</code> - same as <code>\@i[B]</code></li>
 *   <li><code>\@fp[W].[F]</code> - a fixed-point number with W whole bits and F
 *     fractional bits. It displays as a fixed-point number. Example: <code>\@fp0.16</code></li>
 *   <li><code>\@i[B]</code> - an integer with B bits. Example: <code>\@i32</code></li>
 * </ul>
 *
 * @param str    - pointer to a c string pointer that contains the string to parse
 * @param whole  - pointer to where the number of whole bits should be written
 *   (if it's actually an fp)
 *
 * @param frac   - pointer to where the number of frac bits should be written
 *   (if it's actually an fp)
 *
 * @return Whether the string is in fixed point format
 * @note This advances *str if it matches the format.
 * @note For internal use only.
 */
void x3d_vsprintf(char* buf, const char* format, va_list list) {
  const char* str = format;
  char* save_buf = buf;

  while(*str) {  
    if(*str == '@') {
      _Bool is_signed = 1;
      const char* save_str = str;
      int16 whole_bits;
      int16 frac_bits;
      _Bool is_long = 0;
      int32 value;
      int16 repeat = 1;
      char repeat_str[3] = { ' ', 0, 0 };

      ++str;

      if(isdigit(*str)) {
        repeat = *str - '0';
        ++str;

        if(*str == ',') {
          repeat_str[0] = ',';
          repeat_str[1] = ' ';

          ++str;
        }
      }

      const char *begin = str;

process:
      switch(*str) {
      
        /// @TODO - having @@ is broken
      case '@':
        goto normal_char;
        break;

        // The following type is unsigned
      case 'u':
        is_signed = 0;
        ++str;
        goto process;
        break;

        // String
      case 's':
        sprintf(buf, "%s", va_arg(list, char* ));
        ++str;
        break;

        // Integer
      case 'i':
      case 'd':
        ++str;
        int16 size;
        
        parse_int(&str, &size);

        if(size == 0)
          size = 16;

        if(size == 16)
          value = va_arg(list, int);
        else if(size == 32)
          value = va_arg(list, long);
        else {
          goto error;
        }
        
        sprintf(buf, "%ld", value);
        break;

        // fp -> fixed point
      case 'f':
        if(!parse_fp(&str, &whole_bits, &frac_bits)) {
          str = save_str + 1;
          goto normal_char;
        }

        int16 total_bits = whole_bits + frac_bits;

        if(total_bits > 8 && total_bits <= 16) {
          if(is_signed) {
            value = va_arg(list, int);
          }
          else {
            value = va_arg(list, unsigned int);
          }
        }

        // Sign bit has to come from frac bits
        if(whole_bits == 0 && is_signed)
          --frac_bits;

        sprintf(buf, "%f", ((float)value / (1L << frac_bits)));
        break;

      default:
        goto error;
      }

      // Advance over anything we just wrote
      while(*buf)
        ++buf;

      while(--repeat > 0) {
        sprintf(buf, "%s", repeat_str);
        str = begin;

        while(*buf)
          ++buf;

        goto process;
      }
    }
    else {
normal_char:
      *buf++ = *str++;
    }
  }

  return;

error:
  sprintf(buf, "@FORMAT ERR@");
}

void x3d_sprintf(char* buf, const char* format, ...) {
  va_list list;
  va_start(list, format);
  x3d_vsprintf(buf, format, list);
  va_end(list);
}

void x3d_printf(const char* format, ...) {
  va_list list;
  char buf[500];

  va_start(list, format);
  x3d_vsprintf(buf, format, list);
  printf("%s", buf);
  va_end(list);
}