// C Source File
// Created 3/9/2015; 11:23:36 AM

#include "X3D_config.h"
#include "X3D_fix.h"

#ifndef NDEBUG

// Throws an error, prints out the message, and then quits the program
void x3d_error(const char* format, ...) {
  char buf[512];
  va_list list;

  va_start(list, format);
  vsprintf(buf, format, list);

  PortRestore();
  clrscr();
  printf("Error: %s\nPress Esc to quit\n", buf);

  while(!_keytest(RR_ESC));

  exit(-1);
}

int16 hex_digit_to_int16(char digit) {
  if(isdigit(digit))
    return digit - '0';
  else if(digit >= 'A' && digit <= 'G')
    return digit - 'A' + 10;

  // Not a hex digit
  return 0;
}

static _Bool parse_fp(const char** str, int16* whole, int16* frac) {
  const char* s = *str;

  if(s[0] == 'f' && s[1] == 'p' && s[3] == '.') {
    *whole = hex_digit_to_int16(s[2]);
    *frac = hex_digit_to_int16(s[4]);

    *str += 5;
    return 1;
  }

  return 0;
}



void x3d_sprintf(char* buf, const char* format, ...) {
  const char* str = format;
  char* save_buf = buf;

  va_list list;
  va_start(list, format);

  int16 frac_bits = 0;
  int16 whole_bits = 0;

  while(*str) {
    if(*str == '@' && str[1] != '@') {
      ++str;

      if(parse_fp(&str, &frac_bits, &whole_bits)) {
        int16 total_bits = frac_bits + whole_bits;
        int32 value = 0;

        if(total_bits > 8 && total_bits <= 16) {
#ifdef __TIGCC__
          value = va_arg(list, int16);
#else
          value = va_arg(list, int32);
#endif
        }

        sprintf(buf, "%f", ((float)value / (1 << frac_bits)));
        
        while(*buf)
          ++buf;
      }
      else {
        goto normal_char;
      }
    }
    else {
normal_char:
      *buf++ = *str++;
    }
  }

}

#endif

