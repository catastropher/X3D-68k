#include "X3D_config.h"
#include "X3D_fix.h"

int16 hex_digit_to_int16(char digit) {
  if(isdigit(digit))
    return digit - '0';
  else if(digit >= 'A' && digit <= 'G')
    return digit - 'A' + 10;

  // Not a hex digit
  return 0;
}

static _Bool parse_int(const char** str, int16* i) {
  int16 res = 0;

  while(isdigit(**str)) {
    res = (res * 10) + **str - '0';
    (*str)++;
  }

  *i = res;

  return 1;
}

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
















void x3d_sprintf(char* buf, const char* format, ...) {
  const char* str = format;
  char* save_buf = buf;

  va_list list;
  va_start(list, format);
  while(*str) {  
    if(*str == '@') {
      _Bool is_signed = 1;
      const char* save_str = str;
      int16 whole_bits;
      int16 frac_bits;
      _Bool is_long = 0;
      int32 value;

      ++str;

process:
      switch(*str) {
      case '@':
        goto normal_char;
        break;

      case 'u':
        is_signed = 0;
        ++str;
        goto process;
        break;

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
          str = save_str;
          goto normal_char;
        }
        
        sprintf(buf, "%ld", value);
        break;

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
        str = save_str;
        goto normal_char;
      }

      // Advance over anything we just wrote
      while(*buf)
        ++buf;
    }
    else {
normal_char:
      *buf++ = *str++;
    }
  }
}