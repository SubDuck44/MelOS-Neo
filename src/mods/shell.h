#ifndef TERMINAL_SHELL_H
#define TERMINAL_SHELL_H
#pragma once
#include "../main.h"

struct mod_shell {
  uint8_t FontSize;
  uint16_t CaretPosition;
  uint16_t VIndex;
  char** TerminalContent;
  uint16_t NumLines;
};

void modC_shell(void);
inline void modU_shell(uint16_t Index, void* Data);
void modD_shell(RenderTexture2D Canvas, uint16_t Index, void* Data);
void modK_shell(uint16_t Index, struct mod_shell* data);

#endif //TERMINAL_SHELL_H