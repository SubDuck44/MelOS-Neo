#ifndef MELOS_SHELL
#define MELOS_SHELL
#pragma once

#include "../main.h"

struct Shell {
  char* buffer;
  char* lastmessage;
  char* focusedmessage;
  uint8_t fontsize;
  float spacing;
};

struct Shell Shell_Construct(void);
void Shell_Print(struct Shell* target, const char* message, size_tits len);
void Shell_Draw(struct Shell* target);

#endif