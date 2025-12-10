#include "mystring.h"

void mstr_memset(char* src, const size_tits size, const char fill) {
  for (size_tits i = 0; i < size; i++) {
    src[i] = fill;
  }
}

char* mstr_lencpy(const char* src, char* dest, const size_tits size) {
  char* cur_char = dest;
  for (int32_t i = size - 1; i >= 0; i--) {
    *cur_char = src[i];
    cur_char++;
  }
  return cur_char - 1;
}