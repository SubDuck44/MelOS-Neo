#ifndef MELOS_SHELL
#define MELOS_SHELL
#pragma once

#include "../main.h"
#define DEF_SHELLINPUTPLACEHOLDER " "
#define SYS_EVILBUFFERSIZE 65535
#define SHELL_DRAW Sys_Invoke(OP_DRAWCALL, (void*)&(struct Op_DrawCall){(Method)Shell_Draw, target, target->window_index});
#define SHELL_BUFWRAP(var, size) var % size

struct Shell {
  char* buffer;
  uint16_t lastmessage;
  uint16_t caret;
  uint8_t fontsize;
  float spacing;
  uint16_t window_index;
};

struct Shell Shell_Construct(void);
void Shell_Print(struct Shell* target, const char* message, size_tits len);
void Shell_Draw(struct Shell* target);
void Shell_OnInput(struct Shell* target);

#endif
